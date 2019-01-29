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

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeLoader.h>
#include <unity/UnityExceptions.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <core/posix/signal.h>
#include <core/posix/this_process.h>

#include <libgen.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace
{

char const* prog_name;

void error(string const& msg)
{
    assert(!msg.empty());
    cerr << prog_name << ": " << msg << endl;
}

class ThreadWrapper
{
public:
    ThreadWrapper(std::thread t, std::function<void()> cleanup)
        : t_(std::move(t))
        , f_(cleanup)
    {
    }

    ~ThreadWrapper()
    {
        try
        {
            f_();
        }
        catch (...)
        {
        }
        if (t_.joinable())
        {
            t_.join();
        }
    }

private:
    std::thread t_;
    std::function<void()> f_;
};

// Run the scope specified by the config_file in a separate thread and wait for the thread to finish.
// Return exit status for main to use.

int run_scope(std::string const& runtime_config, std::string const& scope_config)
{
    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_hup,
        core::posix::Signal::sig_term
    });

    std::thread trap_worker([trap]{ trap->run(); });
    ThreadWrapper trap_wrapper(std::move(trap_worker), [trap]{ trap->stop(); });

    // Figure out what the scope ID is from the name of the scope config file.
    auto scope_config_path = boost::filesystem::canonical(scope_config);
    string lib_dir = scope_config_path.parent_path().native();
    string scope_id = scope_config_path.stem().native();

    {
        // Make sure we set LD_LIBRARY_PATH to include <lib_dir> and <lib_dir>/lib
        // before loading the scope's .so.
        string scope_ld_lib_path = lib_dir + ":" + lib_dir + "/lib";
        string ld_lib_path = core::posix::this_process::env::get("LD_LIBRARY_PATH", "");
        if (!boost::starts_with(ld_lib_path, lib_dir))
        {
            scope_ld_lib_path = scope_ld_lib_path + (ld_lib_path.empty() ? "" : (":" + ld_lib_path));
            try
            {
                // No overwrite option for this_process::env::set(), need to unset first
                core::posix::this_process::env::unset_or_throw("LD_LIBRARY_PATH");
                core::posix::this_process::env::set_or_throw("LD_LIBRARY_PATH", scope_ld_lib_path);
            }
            catch (std::exception const&)
            {
                throw unity::ResourceException("cannot set LD_LIBRARY_PATH for scope " + scope_id);
            }
        }
    }

    int exit_status = 1;
    try
    {
        // For a scope_id "Fred", we look for the library as "libFred.so", "Fred.so", and "scope.so".
        vector<string> libs;
        libs.push_back(lib_dir + "/" + DEB_HOST_MULTIARCH + "/lib" + scope_id + ".so");
        libs.push_back(lib_dir + "/" + DEB_HOST_MULTIARCH + "/" + scope_id + ".so");
        libs.push_back(lib_dir + "/" + DEB_HOST_MULTIARCH + "/scope.so");
        libs.push_back(lib_dir + "/lib" + scope_id + ".so");
        libs.push_back(lib_dir + "/" + scope_id + ".so");
        libs.push_back(lib_dir + "/scope.so");
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
        trap->signal_raised().connect([&rt](core::posix::Signal)
        {
            lock_guard<mutex> lock(rt_mutex);
            if (rt)
            {
                rt->destroy();
            }
        });

        // Instantiate the run time and run the scope.
        {
            lock_guard<mutex> lock(rt_mutex);
            rt = RuntimeImpl::create(scope_id, runtime_config);
        }

        rt->run_scope(loader->scope_base(), scope_config);

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

    return exit_status;
}

} // namespace

int
main(int argc, char* argv[])
{
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
