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

#include <boost/filesystem/path.hpp>
#include <core/posix/signal.h>

#include <cassert>
#include <future>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <libgen.h>

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

int run_scope(std::string const& runtime_config, std::string const& scope_config)
{
    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_hup,
        core::posix::Signal::sig_term
    });

    std::thread trap_worker([trap]() { trap->run(); });

    // Figure out what the scope ID is from the name of the scope config file.
    filesystem::path scope_config_path(scope_config);
    string lib_dir = scope_config_path.parent_path().native();
    string scope_id = scope_config_path.stem().native();

    if (!lib_dir.empty())
    {
      lib_dir += '/';
    }

    int exit_status = 1;
    try
    {
        // For a scope_id "Fred", we look for the library as "libFred.so", "Fred.so", and "scope.so".
        vector<string> libs;
        libs.push_back(lib_dir + "lib" + scope_id + ".so");
        libs.push_back(lib_dir + scope_id + ".so");
        libs.push_back(lib_dir + "scope.so");
        string failed_libs;
        ScopeLoader::SPtr loader;
        exception_ptr ep;
        for (auto const& lib : libs)
        {
            try
            {
                loader = ScopeLoader::load(scope_id, lib);
            }
            catch (unity::ResourceException& e)
            {
                failed_libs += "\n    " + lib;
                ep = e.remember(ep);
            }
            if (loader)
            {
                break;
            }
        }
        if (!loader)
        {
            unity::ResourceException e("Cannot load scope " + scope_id + "; tried in the following locations:"
                                       + failed_libs);
            e.remember(ep);
            throw e;
        }

        static mutex rt_mutex;
        RuntimeImpl::SPtr rt;

        // Signal handler to shut down the run time on receipt of a signal.
        trap->signal_raised().connect([rt](core::posix::Signal)
        {
            lock_guard<mutex> lock(rt_mutex);
            rt->destroy();
        });

        // Instantiate the run time and run the scope.
        {
            lock_guard<mutex> lock(rt_mutex);
            rt = RuntimeImpl::create(scope_id, runtime_config);
        }

        rt->run_scope(loader->scope_base(), runtime_config, scope_config);

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
