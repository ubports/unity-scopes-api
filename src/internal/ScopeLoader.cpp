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

#include <scopes/internal/ScopeLoader.h>

#include <scopes/Version.h>
#include <unity/UnityExceptions.h>
#include <unity/util/ResourcePtr.h>

#include <cassert>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ScopeLoader::ScopeLoader(string const& name, string const& libpath, RegistryProxy const& registry) :
    scope_name_(name),
    dyn_loader_(DynamicLoader::create(libpath, DynamicLoader::Binding::now, DynamicLoader::Unload::noclose)),
    registry_(registry),
    scope_state_(ScopeState::Created),
    cmd_(ScopeCmd::None),
    run_thread_state_(AppState::Created)
{
    if (!registry)
    {
        throw InvalidArgumentException("Cannot load scope \"" + name + "\": null registry proxy");
    }

    // Look for the scope create and destroy functions in the plug-in.
    CreateFunction create_func = reinterpret_cast<CreateFunction>(
                                     dyn_loader_->find_function(UNITY_API_SCOPE_CREATE_SYMSTR)
                                 );
    DestroyFunction destroy_func = reinterpret_cast<DestroyFunction>(
                                       dyn_loader_->find_function(UNITY_API_SCOPE_DESTROY_SYMSTR)
                                   );

    // Make a new thread. The thread initializes the scope and then waits for commands to transition
    // between the Stopped and Started state, or to the Finished state.
    // We dedicate a separate thread to controlling the scope because
    //  - we can give a guarantee to the scope that create, start(), stop(), and destroy will be
    //    called by the same thread
    //  - if any of the scope-provided methods block for a long time, we don't lose the thread
    //    of control because of a buggy scope
    try
    {
        scope_thread_ = thread(&ScopeLoader::run_scope, this, create_func, destroy_func);
    }
    catch (std::exception const&) // LCOV_EXCL_LINE
    {
        throw ResourceException("Cannot create thread for scope " + name); // LCOV_EXCL_LINE
    }

    // Wait until the thread is running
    unique_lock<mutex> lock(state_mutex_);
    state_changed_.wait(lock, [this] { return scope_state_ != ScopeState::Created; });

    // Check that we initialized successfully.
    if (scope_state_ == ScopeState::Failed)
    {
        lock.unlock();
        scope_thread_.join();
        rethrow_exception(exception_);
    }
}

ScopeLoader::~ScopeLoader() noexcept
{
    try
    {
        unload();
    }
    catch (unity::Exception const& e)
    {
        // TODO: log error
    }
    catch (std::exception const& e) // LCOV_EXCL_LINE
    {
        // TODO: log error
    }
    catch (...) // LCOV_EXCL_LINE
    {
        // TODO: log error
    }
}

ScopeLoader::UPtr ScopeLoader::load(string const& name, string const& libpath, RegistryProxy const& registry)
{
    return UPtr(new ScopeLoader(name, libpath, registry));
}

void ScopeLoader::unload()
{
    std::exception_ptr remembered_exception;
    unique_lock<mutex> lock(state_mutex_);
    switch (scope_state_)
    {
        case ScopeState::Started:
        {
            lock.unlock();
            stop();
            lock.lock();
            state_changed_.wait(lock, [this] { return scope_state_ != ScopeState::Started; });
            if (run_thread_.joinable())
            {
                run_thread_.join();
            }
        }
        // FALLTHROUGH
        case ScopeState::Stopped:
        {
        }
        // FALLTHROUGH
        case ScopeState::Failed:
        {
            remembered_exception.swap(exception_); // We throw only once if unload is called several times.
            {
                // cppcheck-suppress unreadVariable
                lock_guard<mutex> lock(cmd_mutex_);
                cmd_ = ScopeCmd::Finish;
            }
        }
        // FALLTHROUGH
        case ScopeState::Finished:
        {
            cmd_changed_.notify_all();
            lock.unlock();
            if (run_thread_.joinable())            // If unload is called more than once, don't join a second time.
            {
                run_thread_.join();
            }
            if (scope_thread_.joinable())          // If unload is called more than once, don't join a second time.
            {
                scope_thread_.join();
            }
            if (remembered_exception)
            {
                rethrow_exception(remembered_exception);
            }
            break;
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
        }
    }
}

// Send a start command to the scope thread. If the thread is currently started, this call is a no-op.

void ScopeLoader::start()
{
    ScopeState state;
    {
        // cppcheck-suppress unreadVariable
        lock_guard<mutex> lock(state_mutex_);
        state = scope_state_;
    }

    switch (state)
    {
        case ScopeState::Stopped:
        {
            // cppcheck-suppress unreadVariable
            lock_guard<mutex> lock(cmd_mutex_);
            cmd_ = ScopeCmd::Start;
            cmd_changed_.notify_all();
            break;
        }
        case ScopeState::Started:
        {
            break;  // Started already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot start scope \"" + scope_name_ + "\" in Failed state");
        }
        case ScopeState::Finished:
        {
            throw LogicException("Cannot start scope \"" + scope_name_ + "\" in Finished state");
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
        }
    }
}

// Send a stop command to the scope thread. If the thread is currently stopped, this call is a no-op.

void ScopeLoader::stop()
{
    ScopeState state;
    {
        // cppcheck-suppress unreadVariable
        lock_guard<mutex> lock(state_mutex_);
        state = scope_state_;
    }

    switch (state)
    {
        case ScopeState::Started:
        {
            // cppcheck-suppress unreadVariable
            lock_guard<mutex> lock(cmd_mutex_);
            cmd_ = ScopeCmd::Stop;
            cmd_changed_.notify_all();
            break;
        }
        case ScopeState::Stopped:
        case ScopeState::Finished:
        {
            break;  // Stopped already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot stop scope \"" + scope_name_ + "\" in Failed state");
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
        }
    }
}

string ScopeLoader::name() const noexcept
{
    return scope_name_;
}

string ScopeLoader::libpath() const noexcept
{
    return dyn_loader_->path();
}

ScopeBase* ScopeLoader::scope_base() const
{
    assert(scope_base_);
    return scope_base_;
}

// This is the start frame for a separate thread for a scope.
// The thread listens for commands via the cmd_ variable and the cmd_changed_ condition variable.
// The commands cause state transitions between the Started and Stopped state.
//
// A transition to the Started state calls the scope's start() method to tell the scope to get ready.
// A transition to the Stopped state calls the scope's stop() method to tell the scope to finalize itself.
// Transitions between the Stopped and Started state can occur repeatedly.
//
// Once the command to transition to the Finished state arrives, the scope first transitions to the Stopped
// state (if it was in the Started state), and then enters the Finished state, which terminates the scope's thread.
//
// Any errors or exceptions that are thrown by the scope implementation and are not explicitly handled cause
// a transition to the Failed state and terminate the scope thread.

void ScopeLoader::run_scope(CreateFunction create_func, DestroyFunction destroy_func)
{
    try
    {
        // Create the scope instance. If anything goes wrong below, scope_base takes care of destroying it again.
        unity::util::ResourcePtr<ScopeBase*, decltype(destroy_func)> scope_base(create_func(), destroy_func);
        if (scope_base.get() == nullptr)
        {
            // cppcheck-suppress unreadVariable
            unique_lock<mutex> lock(state_mutex_);
            scope_state_ = ScopeState::Failed;
            exception_ = make_exception_ptr(
                             unity::ResourceException("Scope " + scope_name_ +
                                                      " returned nullptr from " + UNITY_API_SCOPE_CREATE_SYMSTR));
            state_changed_.notify_all();
            return;
        }

        // Notify the parent thread that we are ready to go.
        {
            // cppcheck-suppress unreadVariable
            unique_lock<mutex> lock(state_mutex_);
            scope_state_ = ScopeState::Stopped;
            scope_base_ = scope_base.get();
            state_changed_.notify_all();
        }

        // Wait until we are told to do something. We can transition between the Stopped and Started state any
        // number of times. Once we are told to finish, the thread exits, calling the destroy function first.
        bool done = false;
        unique_lock<mutex> lock(cmd_mutex_);
        while (!done)
        {
            cmd_changed_.wait(lock, [this] { return cmd_ != ScopeCmd::None; });
            switch (cmd_)
            {
                case ScopeCmd::Start:
                {
                    lock_guard<mutex> lock(state_mutex_);
                    if (scope_state_ == ScopeState::Started)
                    {
                        break;   // Application thread is running already
                    }

                    // Tell the scope to initialize itself.
                    try
                    {
                        const int s_version = scope_base_->start(scope_name_, registry_);
                        const int maj_version = major_version();
                        if (s_version != maj_version)
                        {
                            scope_state_ = ScopeState::Failed;
                            throw unity::ResourceException("Scope " + scope_name_ +
                                                           " was compiled with major version " +
                                                           to_string(s_version) +
                                                           " of the Unity scopes run time. This " +
                                                           "version is incompatible with the current major " +
                                                           "version (" + to_string(maj_version) + ").");
                        }
                        scope_state_ = ScopeState::Started;
                    }
                    catch (std::exception const& e)
                    {
                        scope_state_ = ScopeState::Failed;
                        exception_ = make_exception_ptr(
                                         unity::ResourceException("Scope " + scope_name_ + ": exception in start()"));
                        throw;
                    }
                    catch (...)
                    {
                        scope_state_ = ScopeState::Failed;
                        exception_ = make_exception_ptr(
                                         unity::ResourceException("Scope " + scope_name_ + ": unknown exception in start()"));
                        throw;
                    }

                    unique_lock<mutex> applock(run_thread_mutex_);
                    run_thread_state_ = AppState::Created;

                    // Pass a thread of control to the scope that it can do with as it likes.
                    run_thread_ = thread(&ScopeLoader::run_application, this, scope_base_);

                    // We wait until the thread leaves the Created state, otherwise it becomes possible
                    // to end up with more than one running application thread if start()/stop()
                    // are called rapidly in succession.
                    run_thread_changed_.wait(applock, [this] { return run_thread_state_ != AppState::Created; });
                    break;
                }
                case ScopeCmd::Stop:
                {
                    unique_lock<mutex> lock(state_mutex_);
                    if (scope_state_ == ScopeState::Stopped)
                    {
                        break;   // Application thread is stopped already
                    }

                    try
                    {
                        scope_base_->stop();
                        scope_state_ = ScopeState::Stopped;
                    }
                    catch (std::exception const& e)
                    {
                        scope_state_ = ScopeState::Failed;
                        exception_ = make_exception_ptr(
                                         unity::ResourceException("scope " + scope_name_ + ": exception in stop()"));
                        throw;
                    }
                    catch (...)
                    {
                        scope_state_ = ScopeState::Failed;
                        exception_ = make_exception_ptr(
                                         unity::ResourceException("scope " + scope_name_ + ": unknown exception in stop()"));
                        throw;
                    }
                    if (run_thread_.joinable())
                    {
                        run_thread_.join();
                    }
                    break;
                }
                case ScopeCmd::Finish:
                {
                    lock_guard<mutex> lock(state_mutex_);
                    scope_state_ = ScopeState::Finished;
                    done = true;
                    break;
                }
                default:
                {
                    assert(false); // LCOV_EXCL_LINE
                }
            }
            state_changed_.notify_all();
            cmd_ = ScopeCmd::None;    // We've dealt with this command
        }
    }
    catch (std::exception const& e)
    {
        exception_ = make_exception_ptr(
                         unity::ResourceException("scope " + scope_name_ + ": terminated due to exception"));
        handle_thread_exception();
    }
    catch (...)
    {
        exception_ = make_exception_ptr(
                         unity::ResourceException("scope " + scope_name_ + ": terminated"));
        handle_thread_exception();
    }
}

void ScopeLoader::handle_thread_exception()
{
    // cppcheck-suppress unreadVariable
    lock_guard<mutex> lock(state_mutex_);
    scope_state_ = ScopeState::Failed;
    state_changed_.notify_all();
}

// This is the start frame of a separate thread. Its only purpose is to hand a thread
// of control to the scope application code. The application code and do with the thread
// whatever it likes, including nothing and just letting run() return.

void ScopeLoader::run_application(ScopeBase* scope)
{

    notify_app_thread_started();
    try
    {
        scope->run();
    }
    catch (...)
    {
        // TODO: log this
        // Run throws, we remember the exception and send a stop command to ourselves, so the stop()
        // callback on the scope will still be called.
        {
            lock_guard<mutex> lock(state_mutex_);
            exception_ = make_exception_ptr(
                             unity::ResourceException("Scope " + scope_name_ + ": exception in run()"));
            scope_state_ = ScopeState::Failed;
            state_changed_.notify_all();
        }

        // cppcheck-suppress unreadVariable
        lock_guard<mutex> lock(cmd_mutex_);
        cmd_ = ScopeCmd::Stop;
        cmd_changed_.notify_all();
    }
}

void ScopeLoader::notify_app_thread_started()
{
    unique_lock<mutex> lock(run_thread_mutex_);
    run_thread_state_ = AppState::Started;
    run_thread_changed_.notify_all();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
