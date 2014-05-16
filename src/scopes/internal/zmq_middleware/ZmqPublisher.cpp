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

#include <unity/scopes/internal/zmq_middleware/ZmqPublisher.h>

#include <unity/scopes/ScopeExceptions.h>

#include <zmqpp/socket.hpp>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

ZmqPublisher::ZmqPublisher(zmqpp::context const* context, std::string const& endpoint, std::string const& topic)
    : context_(context)
    , endpoint_(endpoint)
    , topic_(topic)
    , thread_(std::thread(&ZmqPublisher::publisher_thread, this))
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
            throw MiddlewareException("ZmqPublisher(): publisher_thread failed to start (endpoint: " + endpoint_ + ")");
        }
    }
}

ZmqPublisher::~ZmqPublisher()
{
}

void ZmqPublisher::send_message(std::string const& /*message*/)
{

}

void ZmqPublisher::publisher_thread()
{
    try
    {
        // Create the publisher socket
        zmqpp::socket pub_socket(zmqpp::socket(*context_, zmqpp::socket_type::publish));
        pub_socket.bind(endpoint_);

        // Notify constructor that the thread is now running
        std::unique_lock<std::mutex> lock(mutex_);
        thread_state_ = Running;
        cond_.notify_all();

        // Wait for send_message or stop
        //pub_socket.send(topic_ + message);

        // Clean up
        pub_socket.close();
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_state_ = Failed;
        thread_exception_ = std::current_exception();
        cond_.notify_all();
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
