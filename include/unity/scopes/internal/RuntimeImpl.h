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

#ifndef UNITY_SCOPES_INTERNAL_RUNTIMEIMPL_H
#define UNITY_SCOPES_INTERNAL_RUNTIMEIMPL_H

#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/MiddlewareFactory.h>
#include <unity/scopes/internal/Reaper.h>
#include <unity/scopes/internal/ThreadPool.h>
#include <unity/scopes/Runtime.h>

#include <atomic>

namespace unity
{

namespace scopes
{

namespace internal
{

class RuntimeImpl final
{
public:
    NONCOPYABLE(RuntimeImpl);
    UNITY_DEFINES_PTRS(RuntimeImpl);

    static UPtr create(std::string const& scope_id, std::string const& configfile = "");
    void destroy();

    std::string scope_id() const;
    MiddlewareFactory const* factory() const;
    RegistryProxy registry() const;
    std::string registry_configfile() const;
    std::string registry_identity() const;
    std::string registry_endpointdir() const;
    std::string registry_endpoint() const;
    Reaper::SPtr reply_reaper() const;
    ThreadPool::SPtr pool() const;
    ThreadSafeQueue<std::future<void>>::SPtr future_queue() const;
    void run_scope(ScopeBase *const scope_base);

    ObjectProxy string_to_proxy(std::string const& s) const;
    std::string proxy_to_string(ObjectProxy const& proxy) const;

    ~RuntimeImpl();

private:
    RuntimeImpl(std::string const& scope_id, std::string const& configfile);
    void waiter_thread(ThreadSafeQueue<std::future<void>>::SPtr const& queue) const;

    std::atomic_bool destroyed_;
    std::string scope_id_;
    MiddlewareFactory::UPtr middleware_factory_;
    MiddlewareBase::SPtr middleware_;
    mutable RegistryProxy registry_;
    mutable std::string registry_configfile_;
    mutable std::string registry_identity_;
    mutable std::string registry_endpointdir_;
    mutable std::string registry_endpoint_;
    mutable Reaper::SPtr reply_reaper_;
    mutable ThreadPool::SPtr pool_;                 // Pool of invocation threads for async query creation
    mutable ThreadSafeQueue<std::future<void>>::SPtr future_queue_;
    mutable std::thread waiter_thread_;
    mutable std::mutex mutex_;  // For lazy initialization of reply_reaper_, pool_, and queue_
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
