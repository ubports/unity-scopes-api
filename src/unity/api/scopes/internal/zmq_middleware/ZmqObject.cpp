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

#include <unity/api/scopes/internal/zmq_middleware/ZmqObjectProxy.h>

#include <unity/api/scopes/internal/zmq_middleware/ConnectionPool.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqSender.h>

#include <zmqpp/socket.hpp>

using namespace std;

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

ZmqObjectProxy::ZmqObjectProxy(ZmqMiddleware* mw_base, string const& endpoint, string const& identity, RequestType t) :
    MWObjectProxy(mw_base),
    endpoint_(endpoint),
    identity_(identity),
    type_(t)
{
    assert(!endpoint.empty());
    assert(!identity.empty());
}

ZmqObjectProxy::~ZmqObjectProxy() noexcept
{
}

ZmqMiddleware* ZmqObjectProxy::mw_base() const noexcept
{
    return dynamic_cast<ZmqMiddleware*>(MWObjectProxy::mw_base());
}

string ZmqObjectProxy::endpoint() const
{
    return endpoint_;
}

string ZmqObjectProxy::identity() const
{
    return identity_;
}

RequestType ZmqObjectProxy::type() const
{
    return type_;
}

// Returns a request message with the mode, operation name, endpoint, and identity set for this proxy.

capnproto::Request::Builder ZmqObjectProxy::make_request_(capnp::MessageBuilder& b, std::string const& operation_name) const
{
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(type_ == RequestType::Oneway ? capnproto::RequestMode::ONEWAY : capnproto::RequestMode::TWOWAY);
    request.setOpName(operation_name.c_str());
    request.setId(identity_.c_str());
    return request;
}

// Get a socket to the endpoint for this proxy, write the request on the wire and, if the invocation
// is twoway, return a reader for the response.

unique_ptr<ZmqReceiver> ZmqObjectProxy::invoke_(capnp::MessageBuilder& out_params)
{
    thread_local static ConnectionPool pool(*mw_base()->context());

    zmqpp::socket& s = pool.find(endpoint(), type());
    ZmqSender sender(s);
    sender.send(out_params.getSegmentsForOutput());

    return unique_ptr<ZmqReceiver>(new ZmqReceiver(s));
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
