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
#include <string>
#include <unordered_map>
#include <vector>

#include <libgen.h>

#include <boost/filesystem/path.hpp>

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

// Run the scope specified by the config_file in a separate thread and wait for the thread to finish.
// Return exit status for main to use.

int run_scope(std::string const& runtime_config, std::string const& scope_configfile)
{
    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_hup,
        core::posix::Signal::sig_term
    });

    std::thread trap_worker([trap]() { trap->run(); });

    // Retrieve the registry middleware and create a proxy to its state receiver
    RuntimeConfig rt_config(runtime_config);
    RegistryConfig reg_conf(rt_config.registry_identity(), rt_config.registry_configfile());
    auto reg_runtime = RuntimeImpl::create(rt_config.registry_identity(), runtime_config);
    auto reg_mw = reg_runtime->factory()->find(reg_runtime->registry_identity(), reg_conf.mw_kind());
    auto reg_state_receiver = reg_mw->create_state_receiver_proxy("StateReceiver");

    filesystem::path scope_config_path(scope_configfile);
    string lib_dir = scope_config_path.parent_path().native();
    string scope_id = scope_config_path.stem().native();
    if (!lib_dir.empty())
    {
      lib_dir += '/';
    }

    int exit_status = 1;
    try
    {
        // Instantiate the run time, create the middleware, load the scope from its
        // shared library, and call the scope's start() method.
        auto rt = RuntimeImpl::create(scope_id, runtime_config);
        auto mw = rt->factory()->create(scope_id, reg_conf.mw_kind(), reg_conf.mw_configfile());

        ScopeLoader::SPtr loader = ScopeLoader::load(scope_id, lib_dir + "lib" + scope_id + ".so", rt->registry());
        loader->start();

        // Give a thread to the scope to do with as it likes. If the scope doesn't want to use it and
        // immediately returns from run(), that's fine.
        auto run_future = std::async(launch::async, [loader] { loader->scope_base()->run(); });

        // Create a servant for the scope and register the servant.
        ScopeConfig scope_config(scope_configfile);
        auto scope = unique_ptr<ScopeObject>(new ScopeObject(rt.get(), loader->scope_base()));
        auto proxy = mw->add_scope_object(scope_id, move(scope), scope_config.idle_timeout() * 1000);

        trap->signal_raised().connect([mw](core::posix::Signal)
        {
            mw->stop();
        });

        // Inform the registry that this scope is now ready to process requests
        reg_state_receiver->push_state(scope_id, StateReceiverObject::State::ScopeReady);

        mw->wait_for_shutdown();

        // Inform the registry that this scope is shutting down
        reg_state_receiver->push_state(scope_id, StateReceiverObject::State::ScopeStopping);

        loader->stop();

        // Collect exit status from the run thread. If this throws, the ScopeLoader
        // destructor will still call stop() on the scope.
        run_future.get();

        exit_status = 0;
    }
    catch (std::exception const& e)
    {
        error(e.what());
    }
    catch (...)
    {
        error("unknown exception");
    }

    trap->stop();

    if (trap_worker.joinable())
        trap_worker.join();

    return exit_status;
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
        exit_status = run_scope(runtime_config, scope_config);
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
