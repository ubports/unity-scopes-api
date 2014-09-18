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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQOBJECTPROXY_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQOBJECTPROXY_H

#include <unity/scopes/internal/MWObjectProxy.h>
#include <scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/scopes/internal/zmq_middleware/ConnectionPool.h>
#include <unity/scopes/internal/zmq_middleware/RequestMode.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxyFwd.h>
#include <unity/scopes/internal/zmq_middleware/ZmqRegistryProxyFwd.h>

#include <capnp/message.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ZmqReceiver;

// A Zmq proxy that points at some Zmq object, but without a specific type.

class ZmqObjectProxy : public virtual MWObjectProxy
{
public:

    ZmqObjectProxy(ZmqMiddleware* mw_base,
                   std::string const& endpoint,
                   std::string const& identity,
                   std::string const& category,
                   RequestMode mode,
                   int64_t timeout = -1);
    virtual ~ZmqObjectProxy();

    virtual ZmqMiddleware* mw_base() const noexcept;

    virtual std::string endpoint() const override;
    virtual std::string identity() const override;
    std::string target_category() const;
    RequestMode mode() const;
    virtual int64_t timeout() const noexcept override;

    virtual std::string to_string() const override;

    // Remote operation
    virtual void ping() override;

protected:
    capnproto::Request::Builder make_request_(capnp::MessageBuilder& b, std::string const& operation_name) const;

    void invoke_oneway_(capnp::MessageBuilder& out_params);

    // Holds both the receiver for the unmarshaling buffer (which allocates memory)
    // and the reader that decodes the memory from the unmarshaling buffer.
    // We use unique_ptr because capnp Builder and Reader types are not movable.
    struct TwowayOutParams
    {
        NONCOPYABLE(TwowayOutParams);

        TwowayOutParams() = default;
        TwowayOutParams(TwowayOutParams&&) = default;
        TwowayOutParams& operator=(TwowayOutParams&&) = default;

        std::unique_ptr<ZmqReceiver> receiver;
        std::unique_ptr<capnp::SegmentArrayMessageReader> reader;
    };

    TwowayOutParams invoke_twoway_(capnp::MessageBuilder& in_params);
    TwowayOutParams invoke_twoway_(capnp::MessageBuilder& in_params,
                                   int64_t twoway_timeout,
                                   int64_t locate_timeout = -1);

private:
    TwowayOutParams invoke_twoway__(capnp::MessageBuilder& in_params, int64_t timeout);

    std::string endpoint_;
    std::string identity_;
    std::string category_;
    RequestMode mode_;
    int64_t timeout_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
