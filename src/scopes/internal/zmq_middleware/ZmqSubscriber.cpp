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

#include <unity/scopes/internal/UniqueID.h>
#include <unity/scopes/internal/zmq_middleware/StopPublisher.h>
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

ZmqSubscriber::ZmqSubscriber(zmqpp::context* context, std::string const& publisher_id,
                             std::string const& endpoint_dir, std::string const& topic)
    : context_(context)
    , endpoint_("ipc://" + endpoint_dir + "/" + publisher_id)
    , topic_(topic)
    , thread_state_(NotRunning)
    , thread_exception_(nullptr)
{
    // Validate publisher_id
    if (publisher_id.find('/') != std::string::npos)
    {
        throw MiddlewareException("ZmqSubscriber(): A publisher cannot contain a '/' in its id");
    }

    // Start thread_stopper_ publisher (used to send a stop message to the subscriber on destruction)
    try
    {
        UniqueID unique_id;
        thread_stopper_.reset(new StopPublisher(context, unique_id.gen()));
    }
    catch (...)
    {
        throw MiddlewareException("ZmqSubscriber(): thread_stopper_ failed to initialize (adapter: " + publisher_id + ")");
    }

    // Start the subscriber thread
    thread_ = std::thread(&ZmqSubscriber::subscriber_thread, this);

    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return thread_state_ == Running || thread_state_ == Stopped; });

    if (thread_state_ == Stopped)
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
            throw MiddlewareException("ZmqSubscriber(): subscriber thread failed to start (endpoint: " +
                                      endpoint_ + ")");
        }
    }
}

ZmqSubscriber::~ZmqSubscriber()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_stopper_->stop();
        thread_stopper_->wait_until_stopped();
    }

    if (thread_.joinable())
    {
        thread_.join();
    }
}

std::string ZmqSubscriber::endpoint() const
{
    return endpoint_;
}

void ZmqSubscriber::subscriber_thread()
{
    try
    {
        // Subscribe to our associated publisher socket
        zmqpp::socket sub_socket(*context_, zmqpp::socket_type::subscribe);
        sub_socket.set(zmqpp::socket_option::linger, 0);
        sub_socket.connect(endpoint_);
        sub_socket.subscribe(topic_);

        // Subscribe to the thread_stopper_ socket
        zmqpp::socket stop_socket = thread_stopper_->subscribe();

        // Configure the message poller
        zmqpp::poller poller;
        poller.add(sub_socket);
        poller.add(stop_socket);

        // Notify constructor that the thread is now running
        {
            std::lock_guard<std::mutex> lock(mutex_);
            thread_state_ = Running;
            cond_.notify_all();
        }

        // Poll for messages
        std::string message;
        while (true)
        {
            // poll() throws when the zmq context is destroyed (hence stopping the thread)
            poller.poll();

            // Flush out the message queue before stopping the thread
            if (poller.has_input(sub_socket))
            {
                sub_socket.receive(message);

                // Discard the message if no callback is set
                std::lock_guard<std::mutex> lock(mutex_);
                // Message should arrive in the format: "<topic>:<message>"
                if (message.length() > topic_.length() &&
                    message[topic_.length()] == ':')
                {
                    message_received_(message.substr(topic_.length() + 1));
                }
            }
            else if(poller.has_input(stop_socket))
            {
                break;
            }
        }

        // Clean up
        poller.remove(sub_socket);
        poller.remove(stop_socket);
        sub_socket.close();
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_stopper_->stop();
        thread_exception_ = std::current_exception();
        thread_state_ = Stopped;
        cond_.notify_all();
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
