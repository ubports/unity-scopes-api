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

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQMIDDLEWARE_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQMIDDLEWARE_H

#include <unity/api/scopes/internal/MiddlewareBase.h>
#include <unity/api/scopes/internal/MWRegistryProxyFwd.h>
#include <unity/api/scopes/internal/MWReplyProxyFwd.h>
#include <unity/api/scopes/internal/ThreadPool.h>

#include <zmqpp/context.hpp>

namespace unity
{

namespace api
{

namespace scopes
{

class ScopeBase;

namespace internal
{

class RuntimeImpl;

namespace zmq_middleware
{

class ObjectAdapter;

class ZmqMiddleware final : public MiddlewareBase
{
public:
    ZmqMiddleware(std::string const& server_name, std::string const& configfile, RuntimeImpl* runtime);
    virtual ~ZmqMiddleware() noexcept;

    virtual void start() override;
    virtual void stop() override;
    virtual void wait_for_shutdown() override;

    virtual MWRegistryProxy create_registry_proxy(std::string const& identity, std::string const& endpoint) override;
    virtual MWScopeProxy create_scope_proxy(std::string const& identity, std::string const& endpoint) override;

    virtual MWQueryCtrlProxy add_query_ctrl_object(QueryCtrlObject::SPtr const& ctrl);
    virtual MWQueryProxy add_query_object(QueryObject::SPtr const& query);
    virtual MWRegistryProxy add_registry_object(std::string const& identity, RegistryObject::SPtr const& registry) override;
    virtual MWReplyProxy add_reply_object(ReplyObject::SPtr const& reply) override;
    virtual MWScopeProxy add_scope_object(std::string const& identity, ScopeObject::SPtr const& scope) override;

    // virtual void remove_object(std::string const& identity);
    zmqpp::context* context() const noexcept;
    ThreadPool* invoke_pool() const noexcept;

private:
    std::shared_ptr<ObjectAdapter> find_adapter(std::string const& name);

    std::string server_name_;
    zmqpp::context context_;

    typedef std::map<std::string, std::shared_ptr<ObjectAdapter>> AdapterMap;
    AdapterMap am_;
    mutable std::mutex mutex_;

    std::unique_ptr<ThreadPool> invokers_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
