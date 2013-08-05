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

#ifndef UNITY_API_SCOPES_INTERNAL_SCOPELOADER_H
#define UNITY_API_SCOPES_INTERNAL_SCOPELOADER_H

#include <unity/api/scopes/ScopeBase.h>
#include <unity/api/scopes/internal/DynamicLoader.h>

#include <thread>
#include <condition_variable>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// ScopeLoader loads the .so for a scope and TODO: complete this and updated comments below.

class UNITY_API ScopeLoader final : public util::NonCopyable
{
public:
    UNITY_DEFINES_PTRS(ScopeLoader);

    // Creates a ScopeLoader for a scope with the given name and library. We pass in the registry proxy
    // so we can pass it to the scope's start method.
    static UPtr load(std::string const& scope_name, std::string const& libpath, RegistryProxy::SPtr const& registry);

    // unload() explicitly finalizes the scope. This is called by the destructor too, but calling it explicity
    // allows the caller to receive any exceptions that may have been produced by the scope thread.
    void unload();

    // These methods start or stop a scope. They are asynchronous, that is, they don't wait until the scope is
    // started or stopped; instead they just instruct the scope to do what it is told without waiting.
    void start();
    void stop();

    // Returns the scope name
    std::string name() const noexcept;

    // Returns the library path for the scope
    std::string libpath() const noexcept;

    // Returns the actual implementation provided by the scope.
    unity::api::scopes::ScopeBase* scope_base() const;

    ~ScopeLoader() noexcept;

private:
    ScopeLoader(std::string const& name, std::string const& path, RegistryProxy::SPtr const& registry);
    void run_scope(unity::api::scopes::CreateFunction create_func, unity::api::scopes::DestroyFunction destroy_func);
    void handle_thread_exception();

    void run_application(unity::api::scopes::ScopeBase* scope);
    void notify_app_thread_started();

    std::string scope_name_;
    unity::api::scopes::ScopeBase* scope_base_;
    unity::api::scopes::internal::DynamicLoader::UPtr dyn_loader_;
    unity::api::scopes::RegistryProxy::SPtr registry_;
    std::exception_ptr exception_;

    std::thread scope_thread_;

    enum class ScopeState { Created, Stopped, Started, Finished, Failed };
    ScopeState scope_state_;
    std::condition_variable state_changed_;
    mutable std::mutex state_mutex_;

    enum class ScopeCmd { None, Start, Stop, Finish };
    ScopeCmd cmd_;
    std::condition_variable cmd_changed_;
    std::mutex cmd_mutex_;

    std::thread run_thread_;

    enum class AppState { Created, Started, Stopped };
    AppState run_thread_state_;
    std::condition_variable run_thread_changed_;
    std::mutex run_thread_mutex_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
