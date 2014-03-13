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

#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>

#include <unity/scopes/internal/zmq_middleware/Util.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqSender.h>
#include <unity/scopes/ScopeExceptions.h>

#include <capnp/serialize.h>
#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

#define MONITOR_ENDPOINT "ipc:///tmp/scopes-monitor"

ZmqObjectProxy::ZmqObjectProxy(ZmqMiddleware* mw_base,
                               string const& endpoint,
                               string const& identity,
                               string const& category,
                               RequestMode m,
                               int64_t timeout) :
    MWObjectProxy(mw_base),
    endpoint_(endpoint),
    identity_(identity),
    category_(category),
    mode_(m),
    timeout_(timeout)
{
    assert(m != Unknown);
    assert(timeout >= -1);
    throw_if_bad_endpoint(endpoint);

    // Make sure that fields have consistent settings for null proxies.
    if (endpoint.empty() || identity.empty())
    {
        endpoint_ = "";
        identity_ = "";
        category_ = "";
    }
}

ZmqObjectProxy::~ZmqObjectProxy()
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

string ZmqObjectProxy::category() const
{
    return category_;
}

int64_t ZmqObjectProxy::timeout() const noexcept
{
    return timeout_;
}

string ZmqObjectProxy::to_string() const
{
    if (endpoint_.empty() || identity_.empty())
    {
        return "nullproxy:";
    }
    assert(!endpoint_.empty() && !identity_.empty());
    string s = endpoint_ + "#" + identity_;
    if (!category_.empty())
    {
        s += "!c=" + category_;
    }
    if (mode_ == RequestMode::Oneway)
    {
        s += "!m=o";
    }
    if (timeout_ != -1)
    {
        s += "!t=" + std::to_string(timeout_);
    }
    return s;
}

void ZmqObjectProxy::ping()
{
    capnp::MallocMessageBuilder request_builder;
    make_request_(request_builder, "ping");

    auto future = mw_base()->invoke_pool()->submit([&] { this->invoke_twoway(request_builder); });
    // TODO: dubious, waiter thread in runtimeImpl should do this?
    future.wait();
}

RequestMode ZmqObjectProxy::mode() const
{
    return mode_;
}

// Returns a request message with the mode, operation name, endpoint, and identity set for this proxy.

capnproto::Request::Builder ZmqObjectProxy::make_request_(capnp::MessageBuilder& b, std::string const& operation_name) const
{
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(mode_ == RequestMode::Oneway ? capnproto::RequestMode::ONEWAY : capnproto::RequestMode::TWOWAY);
    request.setOpName(operation_name.c_str());
    request.setId(identity_.c_str());
    request.setCat(category_.c_str());
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
        pool.register_socket(MONITOR_ENDPOINT, move(monitor_socket), RequestMode::Oneway);
    }
}

// Get a socket to the endpoint for this proxy and write the request on the wire.

void ZmqObjectProxy::invoke_oneway(capnp::MessageBuilder& out_params)
{
    // Each calling thread gets its own pool because zmq sockets are not thread-safe.
    thread_local static ConnectionPool pool(*mw_base()->context());

    assert(mode_ == RequestMode::Oneway);
    zmqpp::socket& s = pool.find(endpoint_, mode_);
    ZmqSender sender(s);
    auto segments = out_params.getSegmentsForOutput();
    sender.send(segments);

#ifdef ENABLE_IPC_MONITOR
    if (true) {
        register_monitor_socket(pool, *mw_base()->context());
        zmqpp::socket& monitor = pool.find(MONITOR_ENDPOINT, RequestMode::Oneway);
        auto word_arr = capnp::messageToFlatArray(segments);
        monitor.send_raw(reinterpret_cast<char*>(&word_arr[0]), word_arr.size() * sizeof(capnp::word));
    }
#endif
}

Socket ZmqObjectProxy::invoke_twoway(capnp::MessageBuilder& out_params)
{
    return invoke_twoway(out_params, timeout_);
}

// Get a socket to the endpoint for this proxy and write the request on the wire.
// Poll for the reply with the given timeout.
// Return a socket for the response or throw if the timeout expires.

Socket ZmqObjectProxy::invoke_twoway(capnp::MessageBuilder& out_params, int64_t timeout)
{
    // Each calling thread gets its own pool because zmq sockets are not thread-safe.
    thread_local static ConnectionPool pool(*mw_base()->context());

    assert(mode_ == RequestMode::Twoway);
    Socket s = pool.take(endpoint_, timeout);   // We now have exclusive use of this socket for this invocation
    ZmqSender sender(s.zmqpp_socket());
    auto segments = out_params.getSegmentsForOutput();
    sender.send(segments);

#ifdef ENABLE_IPC_MONITOR
    if (true) {
        register_monitor_socket(pool, *mw_base()->context());
        zmqpp::socket& monitor = pool.find(MONITOR_ENDPOINT, RequestMode::Oneway);
        auto word_arr = capnp::messageToFlatArray(segments);
        monitor.send_raw(reinterpret_cast<char*>(&word_arr[0]), word_arr.size() * sizeof(capnp::word));
    }
#endif

    zmqpp::poller p;
    p.add(s.zmqpp_socket());
    p.poll(timeout);
    if (!p.has_input(s.zmqpp_socket()))
    {
        // If a request times out, we must close the corresponding socket, otherwise
        // zmq gets confused: the reply will never be read, so the socket ends up
        // in a bad state.
        s.invalidate();
        throw TimeoutException("Request timed out after " + std::to_string(timeout) + " milliseconds");
    }
    return s;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
