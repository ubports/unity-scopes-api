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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQMIDDLEWARE_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQMIDDLEWARE_H

#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/MWRegistryProxyFwd.h>
#include <unity/scopes/internal/MWReplyProxyFwd.h>
#include <unity/scopes/internal/ThreadPool.h>
#include <unity/scopes/internal/UniqueID.h>
#include <unity/scopes/internal/zmq_middleware/RequestMode.h>
#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxyFwd.h>
#include <unity/scopes/ObjectProxyFwd.h>

#include <zmqpp/context.hpp>

namespace unity
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
class ServantBase;

class ZmqMiddleware final : public MiddlewareBase
{
public:
    ZmqMiddleware(std::string const& server_name, RuntimeImpl* runtime, std::string const& configfile = "");
    virtual ~ZmqMiddleware();

    virtual void start() override;
    virtual void stop() override;
    virtual void wait_for_shutdown() override;

    virtual ObjectProxy string_to_proxy(std::string const& s) override;
    virtual std::string proxy_to_string(MWProxy const& proxy) override;

    virtual MWRegistryProxy registry_proxy() override;
    virtual MWRegistryProxy ss_registry_proxy() override;

    virtual MWScopeProxy create_scope_proxy(std::string const& identity) override;
    virtual MWScopeProxy create_scope_proxy(std::string const& identity, std::string const& endpoint) override;
    virtual MWQueryProxy create_query_proxy(std::string const& identity, std::string const& endpoint) override;
    virtual MWQueryCtrlProxy create_query_ctrl_proxy(std::string const& identity, std::string const& endpoint) override;
    virtual MWStateReceiverProxy create_state_receiver_proxy(std::string const& identity) override;

    virtual MWQueryCtrlProxy add_query_ctrl_object(QueryCtrlObjectBase::SPtr const& ctrl) override;
    virtual void add_dflt_query_ctrl_object(QueryCtrlObjectBase::SPtr const& ctrl) override;
    virtual MWQueryProxy add_query_object(QueryObjectBase::SPtr const& query) override;
    virtual void add_dflt_query_object(QueryObjectBase::SPtr const& query) override;
    virtual MWRegistryProxy add_registry_object(std::string const& identity, RegistryObjectBase::SPtr const& registry) override;
    virtual MWReplyProxy add_reply_object(ReplyObjectBase::SPtr const& reply) override;
    virtual MWScopeProxy add_scope_object(std::string const& identity, ScopeObjectBase::SPtr const& scope, int64_t idle_timeout) override;
    virtual void add_dflt_scope_object(ScopeObjectBase::SPtr const& scope) override;
    virtual MWStateReceiverProxy add_state_receiver_object(std::string const& identity, StateReceiverObject::SPtr const& state_receiver) override;

    virtual MWPublisher::UPtr create_publisher(std::string const& publisher_id) override;
    virtual MWSubscriber::UPtr create_subscriber(std::string const& publisher_id, std::string const& topic) override;

    virtual std::string get_scope_endpoint() override;
    virtual std::string get_query_endpoint() override;
    virtual std::string get_query_ctrl_endpoint() override;

    zmqpp::context* context() const noexcept;
    ThreadPool* oneway_pool();
    ThreadPool* twoway_pool();
    int64_t locate_timeout() const noexcept;
    int64_t registry_timeout() const noexcept;

private:
    ObjectProxy make_typed_proxy(std::string const& endpoint,
                           std::string const& identity,
                           std::string const& category,
                           RequestMode mode,
                           int64_t timeout);

    std::shared_ptr<ObjectAdapter> find_adapter(std::string const& name, std::string const& endpoint_dir,
                                                std::string const& category, int64_t idle_timeout = -1);

    ZmqProxy safe_add(std::function<void()>& disconnect_func,
                      std::shared_ptr<ObjectAdapter> const& adapter,
                      std::string const& identity,
                      std::shared_ptr<ServantBase> const& servant);

    std::function<void()> safe_dflt_add(std::shared_ptr<ObjectAdapter> const& adapter,
                                        std::string const& category,
                                        std::shared_ptr<ServantBase> const& servant);

    std::string server_name_;
    zmqpp::context context_;
    MWRegistryProxy registry_proxy_;
    MWRegistryProxy ss_registry_proxy_;

    typedef std::map<std::string, std::shared_ptr<ObjectAdapter>> AdapterMap;
    AdapterMap am_;
    std::unique_ptr<ThreadPool> oneway_invoker_;
    std::unique_ptr<ThreadPool> twoway_invokers_;

    mutable std::mutex data_mutex_;             // Protects am_ and the invokers

    UniqueID unique_id_;

    enum State { Created, Started, Stopping, Stopped };
    State state_;
    std::condition_variable state_changed_;
    mutable std::mutex state_mutex_;            // Protects state_
    std::atomic_bool shutdown_flag;

    int64_t twoway_timeout_;                    // Default timeout for twoway invocations
    int64_t locate_timeout_;                    // Timeout for registry locate()
    int64_t registry_timeout_;                  // Timeout for registry operations other than locate()

    std::string public_endpoint_dir_;
    std::string private_endpoint_dir_;
    std::string registry_endpoint_dir_;
    std::string ss_registry_endpoint_dir_;
    std::string registry_identity_;
    std::string ss_registry_identity_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
