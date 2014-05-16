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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/internal/zmq_middleware/ZmqSubscriber.h>

#include <unity/scopes/ScopeExceptions.h>

#include <zmqpp/poller.hpp>
#include <zmqpp/socket.hpp>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

ZmqSubscriber::ZmqSubscriber(zmqpp::context const* context, std::string const& endpoint, std::string const& topic)
    : context_(context)
    , endpoint_(endpoint)
    , topic_(topic)
    , thread_(std::thread(&ZmqSubscriber::subscriber_thread, this))
    , thread_state_(NotRunning)
    , thread_exception_(nullptr)
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return thread_state_ == Running || thread_state_ == Failed; });

    if (thread_state_ == Failed)
    {
        if (thread_.joinable())
        {
            thread_.join();
        }
        try
        {
            std::rethrow_exception(thread_exception_);
        }
        catch (...)
        {
            throw MiddlewareException("ZmqSubscriber(): subscriber thread failed to start (endpoint: " + endpoint_ + ")");
        }
    }
}

ZmqSubscriber::~ZmqSubscriber()
{
}

void ZmqSubscriber::set_message_callback(SubscriberCallback /*callback*/)
{

}

void ZmqSubscriber::subscriber_thread()
{
    try
    {
        // Subscribe to publisher socket
        zmqpp::socket sub_socket(*context_, zmqpp::socket_type::subscribe);
        sub_socket.connect(endpoint_);
        sub_socket.subscribe(topic_);

        // Configure message poller
        zmqpp::poller poller;
        poller.add(sub_socket);

        // Notify constructor that the thread is now running
        std::unique_lock<std::mutex> lock(mutex_);
        thread_state_ = Running;
        cond_.notify_all();

        // Poll for messages
        poller.poll();

        std::string message;
        sub_socket.receive(message);

        // Clean up
        poller.remove(sub_socket);
        sub_socket.close();
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_exception_ = std::current_exception();
        thread_state_ = Failed;
        cond_.notify_all();
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
