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
    : scope_name_(name)
    , dyn_loader_(DynamicLoader::create(path))
    , middleware_(middleware)
    , scope_state_(ScopeState::Created)
    , scope_cmd_(ScopeCmd::None)
{
    // Look for the scope create and destroy functions.
    unity::api::scopes::CreateFunction create_func
        = reinterpret_cast<unity::api::scopes::CreateFunction>(
            dyn_loader_->find_function(UNITY_API_SCOPE_CREATE_SYMSTR)
          );
    unity::api::scopes::DestroyFunction destroy_func
        = reinterpret_cast<unity::api::scopes::DestroyFunction>(
            dyn_loader_->find_function(UNITY_API_SCOPE_DESTROY_SYMSTR)
          );

    // Make a new thread. The thread initializes the scope and then waits for signals to transition
    // between the Stopped and Started state, or to the Finished state.
    scope_thread_ = std::thread(&ScopeLoader::run_scope, this, create_func, destroy_func);

    // Wait until the thread has finished initializing.
    std::unique_lock<std::mutex> lock(mutex_);
    while (scope_state_ == ScopeState::Created)
    {
        state_changed_.wait(lock);
    }

    // Check that we initialized successfully.
    if (scope_state_ == ScopeState::Failed)
    {
        scope_thread_.join();
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
    std::unique_lock<std::mutex> lock(mutex_);
    switch (scope_state_)
    {
        case ScopeState::Started:
        {
            lock.unlock();
            stop();
            lock.lock();
            while (scope_state_ == ScopeState::Started)
            {
                state_changed_.wait(lock);
            }
            // FALLTHROUGH
        }
        case ScopeState::Stopped:
        {
            scope_cmd_ = ScopeCmd::Finish;
            // FALLTHROUGH
        }
        case ScopeState::Failed:
        case ScopeState::Finished:
        {
            lock.unlock();
            cmd_changed_.notify_all();
            if (scope_thread_.joinable())   // If unload is called more than once, don't join a second time.
            {
                scope_thread_.join();
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
    std::unique_lock<std::mutex> lock(mutex_);
    switch (scope_state_)
    {
        case ScopeState::Stopped:
        {
            scope_cmd_ = ScopeCmd::Start;
            cmd_changed_.notify_all();
            break;
        }
        case ScopeState::Started:
        {
            break;  // Started already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot start scope \"" + scope_name_ + "\" in Failed state"); // LCOV_EXCL_LINE
        }
        case ScopeState::Finished:
        {
            throw LogicException("Cannot start scope \"" + scope_name_ + "\" in Finished state"); // LCOV_EXCL_LINE
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
    std::unique_lock<std::mutex> lock(mutex_);
    switch (scope_state_)
    {
        case ScopeState::Started:
        {
            scope_cmd_ = ScopeCmd::Stop;
            cmd_changed_.notify_all();
        }
        case ScopeState::Stopped:
        case ScopeState::Finished:
        {
            break;  // Stopped already
        }
        case ScopeState::Failed:
        {
            throw LogicException("Cannot stop scope \"" + scope_name_ + "\" in Failed state"); // LCOV_EXCL_LINE
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
            std::unique_lock<std::mutex> lock(mutex_);
            scope_state_ = ScopeState::Failed;
            state_changed_.notify_all();
            // TODO: Log error
            return;
        }

        // Notify the parent thread that we are ready to go.
        {
            // cppcheck-suppress unreadVariable
            std::unique_lock<std::mutex> lock(mutex_);
            scope_state_ = ScopeState::Stopped;
            state_changed_.notify_all();
        }

        // Wait until we are told to do something. We can transition between the Stopped and Started state any
        // number of times. Once we are told to finish, the thread exits, calling the destroy function first.
        bool done = false;
        std::unique_lock<std::mutex> lock(mutex_);
        while (!done)
        {
            while (scope_cmd_ == ScopeCmd::None)
            {
                cmd_changed_.wait(lock);
            }
            switch (scope_cmd_)
            {
                case ScopeCmd::Start:
                {
                    scope_base.get()->start();
                    middleware_->start(scope_name_);
                    scope_state_ = ScopeState::Started;
                    state_changed_.notify_all();
                    break;
                }
                case ScopeCmd::Stop:
                {
                    middleware_->stop();
                    scope_base.get()->stop();
                    scope_state_ = ScopeState::Stopped;
                    state_changed_.notify_all();
                    break;
                }
                case ScopeCmd::Finish:
                {
                    scope_state_ = ScopeState::Finished;
                    state_changed_.notify_all();
                    done = true;
                    break;
                }
                default:
                {
                    assert(false); // LCOV_EXCL_LINE
                }
            }
            scope_cmd_ = ScopeCmd::None;    // We've dealt with this command
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
    std::lock_guard<std::mutex> lock(mutex_);
    scope_state_ = ScopeState::Failed;
    state_changed_.notify_all();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
