/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQOBJECTPROXY_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQOBJECTPROXY_H

#include <unity/api/scopes/internal/MWObjectProxy.h>
#include <unity/api/scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/api/scopes/internal/zmq_middleware/RequestType.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqObjectProxyFwd.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqReceiver.h>

#include <capnp/message.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ZmqReceiver;

// An Zmq proxy that points at some Zmq object, but without a specific type.

class ZmqObjectProxy : public virtual MWObjectProxy
{
public:
    ZmqObjectProxy(ZmqMiddleware* mw_base, std::string const& endpoint, std::string const& identity, RequestType t);
    virtual ~ZmqObjectProxy() noexcept;

    virtual ZmqMiddleware* mw_base() const noexcept;

    virtual std::string endpoint() const;
    virtual std::string identity() const;
    RequestType type() const;

protected:
    capnproto::Request::Builder make_request_(capnp::MessageBuilder& b, std::string const& operation_name) const;
    std::unique_ptr<ZmqReceiver> invoke_(capnp::MessageBuilder& out_params);

    // TODO: commented out for now (see below)
#if 0
    template<typename F>
    std::unique_ptr<capnp::SegmentArrayMessageReader> submit_request_(F f);
#endif

private:
    std::string endpoint_;
    std::string identity_;
    RequestType type_;
};

// TODO: Ideally, we would use this template because this is type-independent work, instead of having the
// body of this method repeated in each operation of the derived proxy. But, for some reason, the getRoot
// call below causes a syntax error. Need to try with clang to see whether there is a better error message.
// Or maybe it's a gcc bug...

#if 0
template<typename F>
std::unique_ptr<capnp::SegmentArrayMessageReader> ZmqObjectProxy::submit_request_(F f)
{
    try
    {
        auto future = mw_base()->invoke_pool()->submit(f);
        future.wait();
        auto reader = future.get();
        auto response = reader->getRoot<capnproto::Response>();
        throw_if_runtime_exception(response);
        return reader;
    }
    catch (...)
    {
        throw; // TODO: what to do here?
    }
}
#endif

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
