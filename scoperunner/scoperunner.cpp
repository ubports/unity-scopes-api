/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/MWRegistry.h>
#include <unity/scopes/internal/MWStateReceiver.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RuntimeConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeConfig.h>
#include <unity/scopes/internal/ScopeLoader.h>
#include <unity/scopes/internal/ScopeObject.h>
#include <unity/scopes/internal/ThreadSafeQueue.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <core/posix/signal.h>

#include <cassert>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <libgen.h>

#include <boost/filesystem/path.hpp>

#include <sys/apparmor.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace boost;

namespace
{

char const* prog_name;

void error(string const& msg)
{
    assert(!msg.empty());
    cerr << prog_name << ": " << msg << endl;
}

// One thread for each scope, plus a future that the thread sets when it finishes.

// Run each of the scopes in config_files in a separate thread and wait for each thread to finish.
// Return the number of threads that did not terminate normally.

int run_scope(filesystem::path const& runtime_config, filesystem::path const& scope_config)
{
    auto trap = core::posix::trap_signals_for_process(
    {
        core::posix::Signal::sig_hup,
        core::posix::Signal::sig_term
    });

    std::thread trap_worker([trap]() { trap->run(); });

    // Retrieve the registry middleware and create a proxy to its state receiver
    RuntimeConfig rt_config(runtime_config.native());
    RegistryConfig reg_conf(rt_config.registry_identity(), rt_config.registry_configfile());
    auto reg_runtime = RuntimeImpl::create(rt_config.registry_identity(), runtime_config.native());
    auto reg_mw = reg_runtime->factory()->find(reg_runtime->registry_identity(), reg_conf.mw_kind());
    auto reg_state_receiver = reg_mw->create_state_receiver_proxy("StateReceiver");

    string lib_dir = scope_config.parent_path().native();
    string scope_id = scope_config.stem().native();

    int num_errors = 0;
    try
    {
        auto rt = RuntimeImpl::create(scope_id, runtime_config.native());
        auto mw = rt->factory()->create(scope_id, reg_conf.mw_kind(), reg_conf.mw_configfile());

        ScopeConfig sc(scope_config.c_str());

        // Drop our privileges
        string profile;
        switch (sc.confinement_type())
        {
            case ConfinementType::Trusted:
                break;
            case ConfinementType::UntrustedLocal:
                profile = "unity-scope-local";
                break;
            case ConfinementType::UntrustedInternet:
                profile = "unity-scope-internet";
                break;
        }

        if (!profile.empty())
        {
            int profile_change_code = aa_change_profile(profile.c_str());
            if (profile_change_code != 0)
            {
                ostringstream message;
                message << "Couldn't change to AppArmor profile [" << profile
                        << "] error = [" << profile_change_code << "]";
                error(message.str());
                // FIXME Exit here
            }
        }

        ScopeLoader::SPtr loader = ScopeLoader::load(scope_id, lib_dir + "/lib" + scope_id + ".so", rt->registry());
        loader->start();

        // Give a thread to the scope to do with as it likes. If the scope doesn't want to use it and
        // immediately returns from run(), that's fine.
        auto run_future = std::async(launch::async, [loader] { loader->scope_base()->run(); });

        // Create a servant for the scope and register the servant.
        auto scope = unique_ptr<ScopeObject>(new ScopeObject(rt.get(), loader->scope_base()));
        auto proxy = mw->add_scope_object(scope_id, move(scope));

        trap->signal_raised().connect([loader, mw, reg_state_receiver, scope_id](core::posix::Signal)
        {
            // Inform the registry that this scope is shutting down
            reg_state_receiver->push_state(scope_id, StateReceiverObject::State::ScopeStopping);

            loader->stop();
            mw->stop();
        });

        // Inform the registry that this scope is now ready to process requests
        reg_state_receiver->push_state(scope_id, StateReceiverObject::State::ScopeReady);

        mw->wait_for_shutdown();

        // Collect exit status from the run thread. If this throws, the ScopeLoader
        // destructor will still call stop() on the scope.
        run_future.get();
    }
    catch (std::exception const& e)
    {
        error(e.what());
        ++num_errors;
    }
    catch (...)
    {
        error("unknown exception");
        ++num_errors;
    }

    trap->stop();

    if (trap_worker.joinable())
        trap_worker.join();

    return num_errors;
}

} // namespace

int
main(int argc, char* argv[])
{
    // sig masks are inherited by child processes when forked.
    // we do not want to inherit our parent's (scoperegistry)
    // sig mask, hence we clear it immediately on entry.
    sigset_t set;
    ::sigemptyset(&set);
    ::pthread_sigmask(SIG_SETMASK, &set, nullptr);

    prog_name = basename(argv[0]);
    if (argc != 3)
    {
        cerr << "usage: " << prog_name << " runtime.ini configfile.ini" << endl;
        return 2;
    }
    char const* const runtime_config = argv[1];
    char const* const scope_config = argv[2];

    int exit_status = 1;
    try
    {
        filesystem::path runtime_path(runtime_config);
        if (runtime_path.extension() != ".ini")
        {
            throw ConfigException(string("invalid runtime config file name: \"") + runtime_config + "\": missing .ini extension");
        }

        filesystem::path scope_path(scope_config);
        if (scope_path.extension() != ".ini")
        {
            throw ConfigException(string("invalid scope config file name: \"") + scope_config + "\": missing .ini extension");
        }

        exit_status = run_scope(runtime_path, scope_path);
    }
    catch (std::exception const& e)
    {
        error(e.what());
    }
    catch (string const& e)
    {
        error("fatal error: " + e);
    }
    catch (char const* e)
    {
        error(string("fatal error: ") + e);
    }
    catch (...)
    {
        error("terminated due to unknown exception");
    }

    return exit_status;
}