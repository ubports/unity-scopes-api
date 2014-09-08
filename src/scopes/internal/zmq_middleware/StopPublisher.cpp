/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/internal/zmq_middleware/StopPublisher.h>

#include <unity/scopes/ScopeExceptions.h>

#include <cassert>
#include <iostream>  // TODO: remove this once logging is added

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

StopPublisher::StopPublisher(zmqpp::context* context, string const& inproc_name)
    : context_(context)
    , endpoint_("inproc://" + inproc_name)
    , state_(Starting)
    , stop_sent_(false)
{
    assert(context);
    assert(!inproc_name.empty());

    thread_ = thread(&StopPublisher::stopper_thread, this);

    unique_lock<mutex> lock(m_);
    cond_.wait(lock, [this] { return state_ == Started || state_ == Failed; });

    if (state_ == Failed)
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
        try
        {
            rethrow_exception(ex_);
        }
        catch (...)
        {
            throw MiddlewareException("StopPublisher(): publisher thread failed (endpoint: " + endpoint_ + ")");
        }
    }
}

StopPublisher::~StopPublisher()
{
    stop();
    wait_until_stopped();
    if (thread_.joinable())
    {
        thread_.join();
    }
}

string StopPublisher::endpoint() const
{
    lock_guard<mutex> lock(m_);
    return endpoint_;
}

// Return a socket that becomes ready for reading once stop() is called.
// Once the socket becomes readable, it has exactly one zero-length message to be read.

zmqpp::socket StopPublisher::subscribe()
{
    lock_guard<mutex> lock(m_);

    switch (state_)
    {
        case Stopping:
        case Stopped:
        {
            throw MiddlewareException(
                "StopPublisher::subscribe(): cannot subscribe to stopped publisher "
                "(endpoint: " + endpoint_ + ")");
        }
        // LCOV_EXCL_START
        case Failed:
        {
            try
            {
                rethrow_exception(ex_);
            }
            catch (...)
            {
                throw MiddlewareException(
                    "StopPublisher::subscribe(): cannot subscribe to failed publisher "
                    "(endpoint: " + endpoint_ + ")");
            }
        }
        // LCOV_EXCL_STOP
        default:
        {
            assert(state_ == Started);
            zmqpp::socket s(*context_, zmqpp::socket_type::subscribe);
            s.set(zmqpp::socket_option::linger, 0);
            s.connect(endpoint_);
            s.subscribe("");
            return move(s);
        }
    }
}

// Tell the stopper thread to publish the stop message and terminate itself.
// The stopper thread sends a stop message only once; subsequent calls
// to stop() are no-ops.

void StopPublisher::stop() noexcept
{
    unique_lock<mutex> lock(m_);

    switch (state_)
    {
        case Stopping:
        case Stopped:
        case Failed:
        {
            cond_.notify_all();
            return;  // no-op
        }
        default:
        {
            assert(state_ == Started);
            state_ = Stopping;
            cond_.notify_all();
        }
    }
}

void StopPublisher::wait_until_stopped() noexcept
{
    unique_lock<mutex> lock(m_);
    cond_.wait(lock, [this]{ return state_ == Stopped || state_ == Failed; });
}

void StopPublisher::stopper_thread() noexcept
{
    try
    {
        unique_lock<mutex> lock(m_);

        assert(state_ == Starting);

        // Create the publishing socket.
        zmqpp::socket pub_socket(*context_, zmqpp::socket_type::publish);
        // Allow time for stop message to be buffered, so close() won't discard it.
        pub_socket.set(zmqpp::socket_option::linger, 200);
        pub_socket.bind(endpoint_);

        // Notify that we are ready. This ensures that subscribers
        // cannot subscribe before the pub socket was bound.
        state_ = Started;
        cond_.notify_all();

        lock.unlock();  // Allow parent to wake up.

        // Wait until we are told to stop.
        lock.lock();
        cond_.wait(lock, [this] { return state_ == Stopping; });

        // Write the stop message for the subscribers and close.
        // Sending an empty string is OK; the receiver will get a zero-length message.
        if (!stop_sent_)
        {
            stop_sent_ = true;
            pub_socket.send("");  // Fails if context was terminated
            pub_socket.close();
        }
        state_ = Stopped;
        cond_.notify_all();
    }
    catch (...)
    {
        lock_guard<mutex> lock(m_);
        state_ = Failed;
        ex_ = current_exception();
        cond_.notify_all();
    }

    assert(state_ == Stopped || state_ == Failed);
}

}  // namespace zmq_middleware

}  // namespace internal

}  // namespace scopes

}  // namespace unity
