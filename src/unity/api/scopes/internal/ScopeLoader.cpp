/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/ScopeLoader.h>

#include <unity/api/scopes/Version.h>
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

ScopeLoader::ScopeLoader(string const& name, string const& libpath, RegistryProxy::SPtr const& registry)
    : m_scope_name(name)
    , m_dyn_loader(DynamicLoader::create(libpath, DynamicLoader::Binding::now, DynamicLoader::Unload::noclose))
    , m_registry_(registry)
    , m_scope_state(ScopeState::Created)
    , m_cmd(ScopeCmd::None)
    , m_run_thread_state(AppState::Created)
{
    if (!registry)
    {
        throw InvalidArgumentException("Cannot load scope \"" + name + "\": null registry proxy");
    }

    // Look for the scope create and destroy functions in the plug-in.
    CreateFunction create_func = reinterpret_cast<CreateFunction>(
            m_dyn_loader->find_function(UNITY_API_SCOPE_CREATE_SYMSTR)
    );
    DestroyFunction destroy_func = reinterpret_cast<DestroyFunction>(
            m_dyn_loader->find_function(UNITY_API_SCOPE_DESTROY_SYMSTR)
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
        m_scope_thread = thread(&ScopeLoader::run_scope, this, create_func, destroy_func);
    }
    catch (std::exception const&) // LCOV_EXCL_LINE
    {
        throw ResourceException("Cannot create thread for scope " + name); // LCOV_EXCL_LINE
    }

    // Wait until the thread is running
    unique_lock<mutex> lock(m_state_mutex);
    m_state_changed.wait(lock, [this]{ return m_scope_state != ScopeState::Created; });

    // Check that we initialized successfully.
    if (m_scope_state == ScopeState::Failed)
    {
        lock.unlock();
        m_scope_thread.join();
        rethrow_exception(m_exception);
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

ScopeLoader::UPtr ScopeLoader::load(string const& name, string const& libpath, RegistryProxy::SPtr const& registry)
{
    return UPtr(new ScopeLoader(name, libpath, registry));
}

void ScopeLoader::unload()
{
    std::exception_ptr remembered_exception;
    unique_lock<mutex> lock(m_state_mutex);
    switch (m_scope_state)
    {
        case ScopeState::Started:
        {
            lock.unlock();
            stop();
            lock.lock();
            m_state_changed.wait(lock, [this]{ return m_scope_state != ScopeState::Started; });
            if (m_run_thread.joinable())
            {
                m_run_thread.join();
            }
        }
        // FALLTHROUGH
        case ScopeState::Stopped:
        {
        }
        // FALLTHROUGH
        case ScopeState::Failed:
        {
            remembered_exception.swap(m_exception); // We throw only once if unload is called several times.
            m_cmd = ScopeCmd::Finish;
        }
        // FALLTHROUGH
        case ScopeState::Finished:
        {
            lock.unlock();
            m_cmd_changed.notify_all();
            if (m_run_thread.joinable())            // If unload is called more than once, don't join a second time.
            {
                m_run_thread.join();
            }
            if (m_scope_thread.joinable())          // If unload is called more than once, don't join a second time.
            {
                m_scope_thread.join();
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
        lock_guard<mutex> lock(m_state_mutex);
        state = m_scope_state;
    }

    switch (state)
    {
        case ScopeState::Stopped:
        {
            {
                // cppcheck-suppress unreadVariable
                lock_guard<mutex> lock(m_cmd_mutex);
                m_cmd = ScopeCmd::Start;
            }
            m_cmd_changed.notify_all();
            break;
        }
        case ScopeState::Started:
        {
            break;  // Started already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot start scope \"" + m_scope_name + "\" in Failed state");
        }
        case ScopeState::Finished:
        {
            throw LogicException("Cannot start scope \"" + m_scope_name + "\" in Finished state");
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
        lock_guard<mutex> lock(m_state_mutex);
        state = m_scope_state;
    }

    switch (state)
    {
        case ScopeState::Started:
        {
            {
                // cppcheck-suppress unreadVariable
                lock_guard<mutex> lock(m_cmd_mutex);
                m_cmd = ScopeCmd::Stop;
            }
            m_cmd_changed.notify_all();
        }
        case ScopeState::Stopped:
        case ScopeState::Finished:
        {
            break;  // Stopped already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot stop scope \"" + m_scope_name + "\" in Failed state");
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
        }
    }
}

string ScopeLoader::name() const noexcept
{
    return m_scope_name;
}

string ScopeLoader::libpath() const noexcept
{
    return m_dyn_loader->path();
}

ScopeBase* ScopeLoader::scope_base() const
{
    assert(m_scope_base);
    return m_scope_base;
}

// This is the start frame for a separate thread for a scope.
// The thread listens for commands via the m_cmd variable and the m_cmd_changed condition variable.
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
            {
                // cppcheck-suppress unreadVariable
                unique_lock<mutex> lock(m_state_mutex);
                m_scope_state = ScopeState::Failed;
                m_exception = make_exception_ptr(
                                unity::ResourceException("Scope " + m_scope_name +
                                                         " returned nullptr from " + UNITY_API_SCOPE_CREATE_SYMSTR));
            }
            m_state_changed.notify_all();
            return;
        }

        // Notify the parent thread that we are ready to go.
        {
            {
                // cppcheck-suppress unreadVariable
                unique_lock<mutex> lock(m_state_mutex);
                m_scope_state = ScopeState::Stopped;
                m_scope_base = scope_base.get();
            }
            m_state_changed.notify_all();
        }

        // Wait until we are told to do something. We can transition between the Stopped and Started state any
        // number of times. Once we are told to finish, the thread exits, calling the destroy function first.
        bool done = false;
        unique_lock<mutex> lock(m_cmd_mutex);
        while (!done)
        {
            m_cmd_changed.wait(lock, [this]{ return m_cmd != ScopeCmd::None; });
            switch (m_cmd)
            {
                case ScopeCmd::Start:
                {
                    lock_guard<mutex> lock(m_state_mutex);
                    if (m_scope_state == ScopeState::Started)
                    {
                        break;   // Application thread is running already
                    }

                    // Tell the scope to initialize itself.
                    try
                    {
                        const int s_version = m_scope_base->start(m_registry_);
                        const int maj_version = major_version();
                        if (s_version != maj_version)
                        {
                            m_scope_state = ScopeState::Failed;
                            throw unity::ResourceException("Scope " + m_scope_name +
                                                           " was compiled with major version " +
                                                           to_string(s_version) +
                                                           " of the Unity scopes run time. This " +
                                                           "version is incompatible with the current major " +
                                                           "version (" + to_string(maj_version) + ").");
                        }
                        m_scope_state = ScopeState::Started;
                    }
                    catch (std::exception const& e)
                    {
                        m_scope_state = ScopeState::Failed;
                        m_exception = make_exception_ptr(
                                        unity::ResourceException("Scope " + m_scope_name + ": exception in start()"));
                        throw;
                    }
                    catch (...)
                    {
                        m_scope_state = ScopeState::Failed;
                        m_exception = make_exception_ptr(
                                        unity::ResourceException("Scope " + m_scope_name + ": unknown exception in start()"));
                        throw;
                    }

                    unique_lock<mutex> applock(m_run_thread_mutex);
                    m_run_thread_state = AppState::Created;

                    // Pass a thread of control to the scope that it can do with as it likes.
                    m_run_thread = thread(&ScopeLoader::run_application, this, m_scope_base);

                    // We wait until the thread leaves the Created state, otherwise it becomes possible
                    // to end up with more than one running application thread if start()/stop()
                    // are called rapidly in succession.
                    m_run_thread_changed.wait(applock, [this]{ return m_run_thread_state != AppState::Created; });
                    break;
                }
                case ScopeCmd::Stop:
                {
                    unique_lock<mutex> lock(m_state_mutex);
                    if (m_scope_state == ScopeState::Stopped)
                    {
                        break;   // Application thread is stopped already
                    }

                    try
                    {
                        m_scope_base->stop();
                        m_scope_state = ScopeState::Stopped;
                    }
                    catch (std::exception const& e)
                    {
                        m_scope_state = ScopeState::Failed;
                        m_exception = make_exception_ptr(
                                        unity::ResourceException("scope " + m_scope_name + ": exception in stop()"));
                        throw;
                    }
                    catch (...)
                    {
                        m_scope_state = ScopeState::Failed;
                        m_exception = make_exception_ptr(
                                        unity::ResourceException("scope " + m_scope_name + ": unknown exception in stop()"));
                        throw;
                    }
                    lock.unlock();
                    if (m_run_thread.joinable())
                    {
                        m_run_thread.join();
                    }
                    break;
                }
                case ScopeCmd::Finish:
                {
                    lock_guard<mutex> lock(m_state_mutex);
                    m_scope_state = ScopeState::Finished;
                    done = true;
                    break;
                }
                default:
                {
                    assert(false); // LCOV_EXCL_LINE
                }
            }
            m_state_changed.notify_all();
            m_cmd = ScopeCmd::None;    // We've dealt with this command
        }
    }
    catch (std::exception const& e)
    {
        m_exception = make_exception_ptr(
                        unity::ResourceException("scope " + m_scope_name + ": terminated due to exception"));
        handle_thread_exception();
    }
    catch (...)
    {
        m_exception = make_exception_ptr(
                        unity::ResourceException("scope " + m_scope_name + ": terminated"));
        handle_thread_exception();
    }
}

void ScopeLoader::handle_thread_exception()
{
    {
        // cppcheck-suppress unreadVariable
        lock_guard<mutex> lock(m_state_mutex);
        m_scope_state = ScopeState::Failed;
    }
    m_state_changed.notify_all();
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
        // Run throws, we remember the exception and send a stop command to ourselves, so the stop()
        // callback on the scope will still be called.
        {
            lock_guard<mutex> lock(m_state_mutex);
            m_exception = make_exception_ptr(
                            unity::ResourceException("Scope " + m_scope_name + ": exception in run()"));
            m_scope_state = ScopeState::Failed;
        }
        m_state_changed.notify_all();

        {
            // cppcheck-suppress unreadVariable
            lock_guard<mutex> lock(m_cmd_mutex);
            m_cmd = ScopeCmd::Stop;
        }
        m_cmd_changed.notify_all();
    }
}

void ScopeLoader::notify_app_thread_started()
{
    {
        unique_lock<mutex> lock(m_run_thread_mutex);
        m_run_thread_state = AppState::Started;
    }
    m_run_thread_changed.notify_all();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
