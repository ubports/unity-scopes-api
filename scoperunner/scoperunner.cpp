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
#include <unity/scopes/internal/MWSigReceiver.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
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

bool has_suffix(string const& s, string const& suffix)
{
    auto s_len = s.length();
    auto suffix_len = suffix.length();
    if (s_len >= suffix_len)
    {
        return s.compare(s_len - suffix_len, suffix_len, suffix) == 0;
    }
    return false;
}

string strip_suffix(string const& s, string const& suffix)
{
    auto s_len = s.length();
    auto suffix_len = suffix.length();
    if (s_len >= suffix_len)
    {
        if (s.compare(s_len - suffix_len, suffix_len, suffix) == 0)
        {
            return string(s, 0, s_len - suffix_len);
        }
    }
    return s;
}

// One thread for each scope, plus a future that the thread sets when it finishes.

struct ThreadFuture
{
    thread t;
    std::future<void> f;
};

// Each thread provides its own ID on a queue when it finishes. That allows us to
// then locate the thread in the map. The promise is set by the thread so we can
// find out what happened to it and join with it. Unfortunately, we have to jump
// through these hoops because there is no way to wait on multiple futures in C++ 11.

unordered_map<thread::id, ThreadFuture> threads;

ThreadSafeQueue<thread::id> finished_threads;

// Scope thread start function.

void scope_thread(std::shared_ptr<core::posix::SignalTrap> trap,
                  MWSigReceiverProxy reg_sig_receiver,
                  string const& mw_kind,
                  string const& mw_config,
                  string const& runtime_config,
                  string const& scope_name,
                  string const& lib_dir,
                  promise<void> finished_promise)
{
    try
    {
        // Instantiate the run time, create the middleware, load the scope from its
        // shared library, and call the scope's start() method.
        auto rt = RuntimeImpl::create(scope_name, runtime_config);
        auto mw = rt->factory()->create(scope_name, mw_kind, mw_config);
        ScopeLoader::SPtr loader = ScopeLoader::load(scope_name, lib_dir + "lib" + scope_name + ".so", rt->registry());
        loader->start();

        // Give a thread to the scope to do with as it likes. If the scope doesn't want to use it and
        // immediately returns from run(), that's fine.
        auto run_future = std::async(launch::async, [loader] { loader->scope_base()->run(); });

        // Create a servant for the scope and register the servant.
        auto scope = unique_ptr<ScopeObject>(new ScopeObject(rt.get(), loader->scope_base()));
        auto proxy = mw->add_scope_object(scope_name, move(scope));

        trap->signal_raised().connect([loader, mw, reg_sig_receiver, scope_name](core::posix::Signal)
        {
            // Inform the registry that this scope is shutting down
            reg_sig_receiver->push_signal(scope_name, SigReceiverObject::SignalType::ScopeStopping);

            loader->stop();
            mw->stop();
        });

        // Inform the registry that this scope is now ready to service requests
        reg_sig_receiver->push_signal(scope_name, SigReceiverObject::SignalType::ScopeRunning);

        mw->wait_for_shutdown();

        // Collect exit status from the run thread. If this throws, the ScopeLoader
        // destructor will still call stop() on the scope.
        run_future.get();

        finished_promise.set_value();
    }
    catch (...)
    {
        finished_promise.set_exception(current_exception());
    }

    finished_threads.push(this_thread::get_id());
}

// Run each of the scopes in config_files in a separate thread and wait for each thread to finish.
// Return the number of threads that did not terminate normally.

int run_scopes(string const& runtime_config, vector<string> config_files)
{
    auto trap = core::posix::trap_signals_for_all_subsequent_threads(
    {
        core::posix::Signal::sig_hup,
        core::posix::Signal::sig_term
    });

    std::thread trap_worker([trap]() { trap->run(); });

    // Retrieve the registry middleware and create a proxy to its signal receiver
    auto reg_runtime = RuntimeImpl::create("Registry", runtime_config);
    RegistryConfig reg_conf(reg_runtime->registry_identity(), reg_runtime->registry_configfile());
    auto reg_mw = reg_runtime->factory()->find(reg_runtime->registry_identity(), reg_conf.mw_kind());
    auto reg_sig_receiver = reg_mw->create_sig_receiver_proxy("SigReceiver");

    for (auto file : config_files)
    {
        string file_name = basename(const_cast<char*>(string(file).c_str()));    // basename() modifies its argument
        auto dir_len = file.size() - file_name.size();
        string dir = file.substr(0, dir_len);
        if (*dir.rbegin() != '/')
        {
            dir += "/";
        }
        string scope_name = strip_suffix(file_name, ".ini");

        // For each scope, create a thread that loads the scope and initializes it.
        // Each thread gets a promise to indicate when it is finished. When a thread
        // completes, it fulfils the promise, and pushes its ID onto the finished queue.
        // We collect exit status from the thread via the future from each promise.
        promise<void> p;
        auto f = p.get_future();
        thread t(scope_thread, trap, reg_sig_receiver, reg_conf.mw_kind(), reg_conf.mw_configfile(),
                 runtime_config, scope_name, dir, move(p));

        auto id = t.get_id();
        threads[id] = ThreadFuture { move(t), move(f) };
    }

    // Now wait for the threads to finish (in any order).
    int num_errors = 0;
    for (int i = threads.size(); i > 0; --i)
    {
        try
        {
            auto id = finished_threads.wait_and_pop();
            auto it = threads.find(id);
            assert(it != threads.end());
            it->second.t.join();
            it->second.f.get();             // This will throw if the thread terminated due to an exception
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
    if (argc < 3)
    {
        cerr << "usage: " << prog_name << " runtime.ini configfile.ini [configfile.ini ...]" << endl;
        return 2;
    }
    char const* const runtime_config = argv[1];

    int exit_status = 1;
    try
    {
        vector<string> config_files;
        for (int i = 2; i < argc; ++i)
        {
            if (!has_suffix(argv[i], ".ini"))
            {
                throw ConfigException(string("invalid config file name: \"") + argv[i] + "\": missing .ini extension");
            }
            config_files.push_back(argv[i]);
        }

        exit_status = run_scopes(runtime_config, config_files);
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
