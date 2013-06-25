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
#include <unity/api/scopes/internal/MiddlewareBase.h>

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

class ScopeLoader final : public unity::util::DefinesPtrs<ScopeLoader>
{
public:
    static UPtr load(std::string const& scope_name, std::string const& path, MiddlewareBase::SPtr& middleware);
    void unload() noexcept;

    void start();
    void stop();

    std::string name() const noexcept;
    std::string libpath() const noexcept;

    ~ScopeLoader() noexcept;

private:
    ScopeLoader(std::string const& name, std::string const& path, MiddlewareBase::SPtr& middleware);
    void run_scope(unity::api::scopes::CreateFunction create_func, unity::api::scopes::DestroyFunction destroy_func);
    void handle_thread_exception();

    std::string m_scope_name;
    unity::api::scopes::internal::DynamicLoader::UPtr m_dyn_loader;
    unity::api::scopes::internal::MiddlewareBase::SPtr m_middleware;
    std::thread m_scope_thread;

    enum class ScopeState { Created, Stopped, Started, Finished, Failed };
    ScopeState m_scope_state;
    std::condition_variable m_state_changed;
    enum class ScopeCmd { None, Start, Stop, Finish };
    ScopeCmd m_scope_cmd;
    std::condition_variable m_cmd_changed;
    std::mutex m_mutex;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
