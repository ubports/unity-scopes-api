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
#include <unity/scopes/internal/zmq_middleware/RequestMode.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxyFwd.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReceiver.h>

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
                   RequestMode mode,
                   int64_t timeout = -1);
    virtual ~ZmqObjectProxy() noexcept;

    virtual ZmqMiddleware* mw_base() const noexcept;

    virtual std::string endpoint() const override;
    virtual std::string identity() const override;
    virtual int64_t timeout() const override;
    RequestMode mode() const;

    // Remote operation
    virtual void ping() override;

protected:
    capnproto::Request::Builder make_request_(capnp::MessageBuilder& b, std::string const& operation_name) const;
    ZmqReceiver invoke_(capnp::MessageBuilder& out_params);
    ZmqReceiver invoke_(capnp::MessageBuilder& out_params, int64_t timeout);

private:
    std::string endpoint_;
    std::string identity_;
    int64_t timeout_;
    RequestMode mode_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
