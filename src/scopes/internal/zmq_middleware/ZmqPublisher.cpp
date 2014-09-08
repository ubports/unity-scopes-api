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

#include <unity/scopes/internal/zmq_middleware/Util.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/util/ResourcePtr.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

ZmqPublisher::ZmqPublisher(zmqpp::context* context, std::string const& publisher_id,
                           std::string const& endpoint_dir)
    : context_(context)
    , endpoint_("ipc://" + endpoint_dir + "/" + publisher_id)
    , thread_state_(NotRunning)
    , thread_exception_(nullptr)
{
    // Validate publisher_id
    if (publisher_id.find('/') != std::string::npos)
    {
        throw MiddlewareException("ZmqPublisher(): A publisher cannot contain a '/' in its id");
    }

    // Start the publisher thread
    thread_ = std::thread(&ZmqPublisher::publisher_thread, this);

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
            throw MiddlewareException("ZmqPublisher(): publisher thread failed to start (endpoint: " +
                                      endpoint_ + ")");
        }
    }
}

ZmqPublisher::~ZmqPublisher()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_state_ = Stopping;
        cond_.notify_all();
    }

    if (thread_.joinable())
    {
        thread_.join();
    }
}

std::string ZmqPublisher::endpoint() const
{
    return endpoint_;
}

void ZmqPublisher::send_message(std::string const& message, std::string const& topic)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Write message in the format: "<topic>:<message>"
    message_queue_.push(topic + ':' + message);
    cond_.notify_all();
}

void ZmqPublisher::publisher_thread()
{
    try
    {
        // Create the publisher socket
        zmqpp::socket pub_socket(*context_, zmqpp::socket_type::publish);
        pub_socket.set(zmqpp::socket_option::linger, 50);
        safe_bind(pub_socket, endpoint_);

        // Notify constructor that the thread is now running
        std::unique_lock<std::mutex> lock(mutex_);
        thread_state_ = Running;
        cond_.notify_all();

        // Wait for send_message or stop
        while (true)
        {
            // mutex_ unlocked
            cond_.wait(lock, [this] { return thread_state_ == Stopping || !message_queue_.empty(); });
            // mutex_ locked

            // Flush out the message queue before stopping the thread
            if (!message_queue_.empty())
            {
                pub_socket.send(message_queue_.front());
                message_queue_.pop();
            }
            else if (thread_state_ == Stopping)
            {
                break;
            }
        }

        // Clean up
        pub_socket.close();
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
