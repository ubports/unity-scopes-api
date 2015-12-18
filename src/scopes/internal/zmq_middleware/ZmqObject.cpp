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

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/zmq_middleware/Util.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReceiver.h>
#include <unity/scopes/internal/zmq_middleware/ZmqRegistry.h>
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

namespace
{
    // this mutex protects all members of all ZmqObjectProxies
    std::mutex shared_mutex;
}

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
    lock_guard<mutex> lock(shared_mutex);
    return endpoint_;
}

string ZmqObjectProxy::identity() const
{
    lock_guard<mutex> lock(shared_mutex);
    return identity_;
}

string ZmqObjectProxy::target_category() const
{
    lock_guard<mutex> lock(shared_mutex);
    return category_;
}

int64_t ZmqObjectProxy::timeout() const noexcept
{
    lock_guard<mutex> lock(shared_mutex);
    return timeout_;
}

string ZmqObjectProxy::to_string() const
{
    lock_guard<mutex> lock(shared_mutex);
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

    auto future = mw_base()->twoway_pool()->submit([&] { return this->invoke_twoway_(request_builder); });

    auto out_params = future.get();
    auto response = out_params.reader->getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);
}

RequestMode ZmqObjectProxy::mode() const
{
    lock_guard<mutex> lock(shared_mutex);
    return mode_;
}

// Returns a request message with the mode, operation name, endpoint, and identity set for this proxy.

capnproto::Request::Builder ZmqObjectProxy::make_request_(capnp::MessageBuilder& b, std::string const& operation_name) const
{
    lock_guard<mutex> lock(shared_mutex);
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(mode_ == RequestMode::Oneway ? capnproto::RequestMode::ONEWAY : capnproto::RequestMode::TWOWAY);
    request.setOpName(operation_name.c_str());
    request.setId(identity_.c_str());
    request.setCat(category_.c_str());
    return request;
}

// Get a socket to the endpoint for this proxy and write the request on the wire.

void ZmqObjectProxy::invoke_oneway_(capnp::MessageBuilder& request)
{
    // Each calling thread gets its own pool because zmq sockets are not thread-safe.
    thread_local static ConnectionPool pool(*mw_base()->context());

    lock_guard<mutex> lock(shared_mutex);

    assert(mode_ == RequestMode::Oneway);
    shared_ptr<zmqpp::socket> s = pool.find(endpoint_);
    ZmqSender sender(*s);
    auto segments = request.getSegmentsForOutput();
    trace_request_(request);
    if (!sender.send(segments, ZmqSender::DontWait))
    {
        // If there is nothing at the other end, discard the message and trash the socket.
        pool.remove(endpoint_);
        return;
    }
}

ZmqObjectProxy::TwowayOutParams ZmqObjectProxy::invoke_twoway_(capnp::MessageBuilder& request)
{
    return invoke_twoway_(request, timeout_, mw_base()->locate_timeout());
}

ZmqObjectProxy::TwowayOutParams ZmqObjectProxy::invoke_twoway_(capnp::MessageBuilder& request,
                                                               int64_t twoway_timeout)
{
    return invoke_twoway_(request, twoway_timeout, mw_base()->locate_timeout());
}

ZmqObjectProxy::TwowayOutParams ZmqObjectProxy::invoke_twoway_(capnp::MessageBuilder& request,
                                                               int64_t twoway_timeout,
                                                               int64_t locate_timeout)
{
    auto registry_proxy = mw_base()->registry_proxy();
    auto ss_registry_proxy = mw_base()->ss_registry_proxy();

    // TODO: HACK: this builds knowledge about the smartscopes proxy running permanently into the run time.
    bool this_is_registry = registry_proxy && identity() == registry_proxy->identity();
    bool this_is_ss_registry = ss_registry_proxy && identity() == ss_registry_proxy->identity();

    // If a registry is configured and this object is not a registry itself,
    // attempt to locate the scope before invoking it.
    if (registry_proxy && !this_is_registry && !this_is_ss_registry)
    {
        try
        {
            ObjectProxy new_proxy;
            new_proxy = registry_proxy->locate(identity(), locate_timeout);

            // update our proxy with the newly received data
            // (we need to first store values in local variables outside of the mutex,
            // otherwise we will deadlock on the following ZmqObjectProxy methods)
            std::string endpoint = new_proxy->endpoint();
            std::string identity = new_proxy->identity();
            std::string category = new_proxy->target_category();
            int64_t timeout = new_proxy->timeout();
            {
                lock_guard<mutex> lock(shared_mutex);
                endpoint_ = endpoint;
                identity_ = identity;
                category_ = category;
                timeout_ = timeout;
            }
        }
        catch (NotFoundException const&)
        {
            // Ignore a failed locate() for scopes unknown to the registry
        }
    }

    // Try the invocation
    return invoke_twoway__(request, twoway_timeout);
}

// Get a socket to the endpoint for this proxy and write the request on the wire.
// Poll for the reply with the given timeout.
// Return a reader for the response or throw if the timeout expires.

ZmqObjectProxy::TwowayOutParams ZmqObjectProxy::invoke_twoway__(capnp::MessageBuilder& request, int64_t timeout)
{
    std::string endpoint;
    {
        lock_guard<mutex> lock(shared_mutex);
        endpoint = endpoint_;
        assert(mode_ == RequestMode::Twoway);
    }

    zmqpp::socket s(*mw_base()->context(), zmqpp::socket_type::request);
    // Allow some linger time so we don't hang indefinitely if the other end disappears.
    s.set(zmqpp::socket_option::linger, 100);
    // We set a reconnect interval of 20 ms, so we get to the peer quickly, in case
    // the peer hasn't finished binding to its endpoint yet after being exec'd.
    // We back off exponentially to half the call timeout. If we haven't connected
    // by then, the poll below will time out anyway. For infinite timeout, we try
    // once a second.
    int reconnect_max = timeout == -1 ? 1000 : timeout / 2;
    s.set(zmqpp::socket_option::reconnect_interval, 20);
    s.set(zmqpp::socket_option::reconnect_interval_max, reconnect_max);
    s.connect(endpoint);
    ZmqSender sender(s);
    auto segments = request.getSegmentsForOutput();
    trace_request_(request);
    sender.send(segments);

    zmqpp::poller p;
    p.add(s);

    if (timeout == -1)
    {
        p.poll();
    }
    else
    {
        p.poll(timeout);
    }

    if (!p.has_input(s))
    {
        string op_name = request.getRoot<capnproto::Request>().getOpName().cStr();
        throw TimeoutException("Request timed out after " + std::to_string(timeout) + " milliseconds (endpoint = " +
                               endpoint + ", op = " + op_name + ")");
    }

    // Because the ZmqReceiver holds the memory for the unmarshaling buffer, we pass both the receiver
    // and the capnp reader in a struct.
    ZmqObjectProxy::TwowayOutParams out_params;
    out_params.receiver.reset(new ZmqReceiver(s));
    auto params = out_params.receiver->receive();
    out_params.reader.reset(new capnp::SegmentArrayMessageReader(params));
    trace_reply_(request, *out_params.reader);
    return out_params;
    // Outgoing twoway socket closed here.
}

string ZmqObjectProxy::decode_request_(capnp::MessageBuilder& request)
{
    auto r = request.getRoot<capnproto::Request>();
    auto mode = r.getMode();
    auto op_name = r.getOpName().cStr();
    auto identity = r.getId().cStr();
    auto cat = r.getCat().cStr();
    stringstream s;
    s << "op = " << op_name
      << ", id = " << identity
      << ", cat = " << cat
      << ", mode = " << (mode == capnproto::RequestMode::ONEWAY ? "oneway" : "twoway");
    return s.str();
}

void ZmqObjectProxy::trace_request_(capnp::MessageBuilder& request)
{
    BOOST_LOG(mw_base()->runtime()->logger(Logger::IPC))
        << "sending request: "
        << decode_request_(request);
}

string ZmqObjectProxy::decode_reply_(capnp::MessageBuilder& request, capnp::MessageReader& reply)
{
    auto response = reply.getRoot<capnproto::Response>();
    return decode_request_(request) + ", " + decode_status(response);
}

void ZmqObjectProxy::trace_reply_(capnp::MessageBuilder& request, capnp::MessageReader& reply)
{
    BOOST_LOG(mw_base()->runtime()->logger(Logger::IPC))
        << "received reply: "
        << decode_reply_(request, reply);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
