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

#include <scopes/internal/zmq_middleware/ZmqObjectProxy.h>

#include <scopes/internal/zmq_middleware/ConnectionPool.h>
#include <scopes/internal/zmq_middleware/ZmqException.h>
#include <scopes/internal/zmq_middleware/ZmqSender.h>

#include <zmqpp/socket.hpp>
#include <capnp/serialize.h>

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

#define MONITOR_ENDPOINT "ipc:///tmp/scopes-monitor"

ZmqObjectProxy::ZmqObjectProxy(ZmqMiddleware* mw_base, string const& endpoint, string const& identity) :
    MWObjectProxy(mw_base),
    endpoint_(endpoint),
    identity_(identity),
    type_(Unknown)
{
    assert(!endpoint.empty());
    assert(!identity.empty());
}

ZmqObjectProxy::ZmqObjectProxy(ZmqMiddleware* mw_base, string const& endpoint, string const& identity, RequestType t) :
    MWObjectProxy(mw_base),
    endpoint_(endpoint),
    identity_(identity),
    type_(t)
{
    assert(!endpoint.empty());
    assert(!identity.empty());
    assert(t != Unknown);
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

void ZmqObjectProxy::ping()
{
    capnp::MallocMessageBuilder request_builder;
    make_request_(request_builder, "ping");

    auto future = mw_base()->invoke_pool()->submit([&] { this->invoke_(request_builder); });
    future.wait();
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

void register_monitor_socket (ConnectionPool& pool, zmqpp::context_t const& context)
{
    thread_local static bool monitor_initialized = false;
    if (!monitor_initialized) {
        monitor_initialized = true;
        zmqpp::socket monitor_socket(context, zmqpp::socket_type::publish);
        monitor_socket.connect(MONITOR_ENDPOINT);
        monitor_socket.set(zmqpp::socket_option::linger, 0);
        pool.register_socket(MONITOR_ENDPOINT, move(monitor_socket), RequestType::Oneway);
    }
}

// Get a socket to the endpoint for this proxy, write the request on the wire and, if the invocation
// is twoway, return a reader for the response.

unique_ptr<ZmqReceiver> ZmqObjectProxy::invoke_(capnp::MessageBuilder& out_params)
{
    thread_local static ConnectionPool pool(*mw_base()->context());

    zmqpp::socket& s = pool.find(endpoint(), type());
    ZmqSender sender(s);
    auto segments = out_params.getSegmentsForOutput();
    sender.send(segments);

#ifdef ENABLE_IPC_MONITOR
    if (true) {
        register_monitor_socket(pool, *mw_base()->context());
        zmqpp::socket& monitor = pool.find(MONITOR_ENDPOINT, RequestType::Oneway);
        auto word_arr = capnp::messageToFlatArray(segments);
        monitor.send_raw(reinterpret_cast<char*>(&word_arr[0]), word_arr.size() * sizeof(capnp::word));
    }
#endif

    return unique_ptr<ZmqReceiver>(new ZmqReceiver(s));
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
