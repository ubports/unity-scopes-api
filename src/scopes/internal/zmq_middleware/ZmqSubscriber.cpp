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

ZmqSubscriber::ZmqSubscriber(zmqpp::context* context, std::string const& endpoint, std::string const& topic)
    : context_(context)
    , endpoint_(endpoint)
    , topic_(topic)
    , thread_(std::thread(&ZmqSubscriber::subscriber_thread, this))
{
}

ZmqSubscriber::~ZmqSubscriber()
{
}

void ZmqSubscriber::set_message_callback(SubscriberCallback /*callback*/)
{

}

void ZmqSubscriber::subscriber_thread()
{
    // Subscribe to publisher socket
    zmqpp::socket sub_socket(*context_, zmqpp::socket_type::subscribe);
    sub_socket.connect(endpoint_);
    sub_socket.subscribe(topic_);

    // Poll for messages
    std::string message;
    zmqpp::poller poller;
    poller.add(sub_socket);
    poller.poll();
    sub_socket.receive(message);

    // Clean up
    poller.remove(sub_socket);
    sub_socket.close();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
