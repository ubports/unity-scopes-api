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
#include <unity/api/scopes/ScopeBase.h>
#include <unity/UnityExceptions.h>
#include <unity/util/ResourcePtr.h>

#include <cassert>

using namespace std;
using namespace unity::api::scopes::internal;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ScopeLoader::ScopeLoader(string const& name, string const& path, MiddlewareBase::SPtr& middleware)
    : m_scope_name(name)
    , m_dyn_loader(DynamicLoader::create(path))
    , m_middleware(middleware)
    , m_scope_state(ScopeState::Created)
    , m_scope_cmd(ScopeCmd::None)
{
    // Look for the scope create and destroy functions.
    unity::api::scopes::CreateFunction create_func
        = reinterpret_cast<unity::api::scopes::CreateFunction>(
            m_dyn_loader->find_function(UNITY_API_SCOPE_CREATE_SYMSTR)
          );
    unity::api::scopes::DestroyFunction destroy_func
        = reinterpret_cast<unity::api::scopes::DestroyFunction>(
            m_dyn_loader->find_function(UNITY_API_SCOPE_DESTROY_SYMSTR)
          );

    // Make a new thread. The thread initializes the scope and then waits for signals to transition
    // between the Stopped and Started state, or to the Finished state.
    m_scope_thread = std::thread(&ScopeLoader::run_scope, this, create_func, destroy_func);

    // Wait until the thread has finished initializing.
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_scope_state == ScopeState::Created)
    {
        m_state_changed.wait(lock);
    }

    // Check that we initialized successfully.
    if (m_scope_state == ScopeState::Failed)
    {
        m_scope_thread.join();
        throw unity::ResourceException("Failed to initialize thread for scope \"" + name + "\"");
    }
}

ScopeLoader::~ScopeLoader() noexcept
{
    unload();
}

ScopeLoader::UPtr ScopeLoader::load(string const& name, string const& path, MiddlewareBase::SPtr& middleware)
{
    return UPtr(new ScopeLoader(name, path, middleware));
}

void ScopeLoader::unload() noexcept
{
    std::unique_lock<std::mutex> lock(m_mutex);
    switch (m_scope_state)
    {
        case ScopeState::Started:
        {
            lock.unlock();
            stop();
            lock.lock();
            while (m_scope_state == ScopeState::Started)
            {
                m_state_changed.wait(lock);
            }
            // FALLTHROUGH
        }
        case ScopeState::Stopped:
        {
            m_scope_cmd = ScopeCmd::Finish;
            // FALLTHROUGH
        }
        case ScopeState::Failed:
        case ScopeState::Finished:
        {
            lock.unlock();
            m_cmd_changed.notify_all();
            if (m_scope_thread.joinable())   // If unload is called more than once, don't join a second time.
            {
                m_scope_thread.join();
            }
            break;
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
        }
    }
}

void ScopeLoader::start()
{
    // cppcheck-suppress unreadVariable
    std::unique_lock<std::mutex> lock(m_mutex);
    switch (m_scope_state)
    {
        case ScopeState::Stopped:
        {
            m_scope_cmd = ScopeCmd::Start;
            m_cmd_changed.notify_all();
            break;
        }
        case ScopeState::Started:
        {
            break;  // Started already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot start scope \"" + m_scope_name + "\" in Failed state"); // LCOV_EXCL_LINE
        }
        case ScopeState::Finished:
        {
            throw LogicException("Cannot start scope \"" + m_scope_name + "\" in Finished state"); // LCOV_EXCL_LINE
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
        }
    }
}

void ScopeLoader::stop()
{
    // cppcheck-suppress unreadVariable
    std::unique_lock<std::mutex> lock(m_mutex);
    switch (m_scope_state)
    {
        case ScopeState::Started:
        {
            m_scope_cmd = ScopeCmd::Stop;
            m_cmd_changed.notify_all();
        }
        case ScopeState::Stopped:
        case ScopeState::Finished:
        {
            break;  // Stopped already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot stop scope \"" + m_scope_name + "\" in Failed state"); // LCOV_EXCL_LINE
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

void ScopeLoader::run_scope(unity::api::scopes::CreateFunction create_func,
                            unity::api::scopes::DestroyFunction destroy_func)
{
    try
    {
        // Create the scope instance. If anything goes wrong below, scope_base takes care of destroying it again.
        unity::util::ResourcePtr<ScopeBase*, decltype(destroy_func)> scope_base(create_func(), destroy_func);
        if (scope_base.get() == nullptr)
        {
            // cppcheck-suppress unreadVariable
            std::unique_lock<std::mutex> lock(m_mutex);
            m_scope_state = ScopeState::Failed;
            m_state_changed.notify_all();
            // TODO: Log error
            return;
        }

        // Notify the parent thread that we are ready to go.
        {
            // cppcheck-suppress unreadVariable
            std::unique_lock<std::mutex> lock(m_mutex);
            m_scope_state = ScopeState::Stopped;
            m_state_changed.notify_all();
        }

        // Wait until we are told to do something. We can transition between the Stopped and Started state any
        // number of times. Once we are told to finish, the thread exits, calling the destroy function first.
        bool done = false;
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!done)
        {
            while (m_scope_cmd == ScopeCmd::None)
            {
                m_cmd_changed.wait(lock);
            }
            switch (m_scope_cmd)
            {
                case ScopeCmd::Start:
                {
                    scope_base.get()->start();
                    m_middleware->start(m_scope_name);
                    m_scope_state = ScopeState::Started;
                    m_state_changed.notify_all();
                    break;
                }
                case ScopeCmd::Stop:
                {
                    m_middleware->stop();
                    scope_base.get()->stop();
                    m_scope_state = ScopeState::Stopped;
                    m_state_changed.notify_all();
                    break;
                }
                case ScopeCmd::Finish:
                {
                    m_scope_state = ScopeState::Finished;
                    m_state_changed.notify_all();
                    done = true;
                    break;
                }
                default:
                {
                    assert(false); // LCOV_EXCL_LINE
                }
            }
            m_scope_cmd = ScopeCmd::None;    // We've dealt with this command
        }
    }
    catch (unity::Exception const& e)
    {
        handle_thread_exception();
        // TODO Log error somewhere
        e.to_string();
    }
    catch (std::exception const& e)
    {
        handle_thread_exception();
        // TODO Log error somewhere
        e.what();
    }
    catch (...)
    {
        handle_thread_exception();
        // TODO: Log error somewhere
    }
}

void ScopeLoader::handle_thread_exception()
{
    // cppcheck-suppress unreadVariable
    std::lock_guard<std::mutex> lock(m_mutex);
    m_scope_state = ScopeState::Failed;
    m_state_changed.notify_all();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
