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
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/zmq_middleware/ConnectionPool.h>

#include <zmqpp/context.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes::internal::zmq_middleware;

// Basic test.

TEST(ConnectionPool, basic)
{
    zmqpp::context context;
    ConnectionPool pool(context);

    auto s = pool.find("ipc:///tmp/test_socket");
    EXPECT_TRUE(s.get());

    // find() for an already connected socket must
    // return the same physical socket.
    auto s2 = pool.find("ipc:///tmp/test_socket");
    EXPECT_EQ(s.get(), s2.get());

    // find() for a different endpoint must
    // return a different socket.
    auto s3 = pool.find("ipc:///tmp/other_socket");
    EXPECT_NE(s.get(), s3.get());

    // Remember the address of s3, remove s3 from the pool,
    // and destroy s3, which closes the socket.
    // Then try to retrieve s3's socket type. That attempt must
    // fail because the socket no longer exists.
    pool.remove("ipc:///tmp/other_socket");
    void* sp = static_cast<void*>(*s3);         // calls operator void*() on zmqpp::socket
    s3 = nullptr;                               // Null out s3 to close the socket.

    int val;
    size_t val_size = sizeof(val);
    EXPECT_EQ(-1, zmq_getsockopt(sp, ZMQ_TYPE, &val, &val_size));

    // Removing a non-existent socket does nothing.
    pool.remove("ipc:///tmp/no_such_socket");
}

TEST(ConnectionPool, reap)
{
    zmqpp::context context;
    ConnectionPool pool(context, 1);

    auto s = pool.find("ipc:///tmp/test_socket");
    EXPECT_TRUE(s.get());
    void* sp = static_cast<void*>(*s);
    s = nullptr;

    // Socket must be alive.
    int val;
    size_t val_size = sizeof(val);
    EXPECT_EQ(0, zmq_getsockopt(sp, ZMQ_TYPE, &val, &val_size));

    this_thread::sleep_for(chrono::seconds(2));

    // Socket must have been closed by reaper.
    val_size = sizeof(val);
    EXPECT_EQ(-1, zmq_getsockopt(sp, ZMQ_TYPE, &val, &val_size));
}

TEST(ConnectionPool, register_socket)
{
    zmqpp::context context;
    ConnectionPool pool(context, 1);

    char const* endpoint = "ipc:///tmp/some_socket";

    // Add a socket that won't be reaped.
    auto socket = make_shared<zmqpp::socket>(context, zmqpp::socket_type::push);
    socket->connect(endpoint);
    pool.register_socket(endpoint, socket, false);
    void* sp = static_cast<void*>(*socket);
    socket = nullptr;

    this_thread::sleep_for(chrono::seconds(2));

    // Socket must still be alive because reaping was disabled by call to register_socket.
    int val;
    size_t val_size = sizeof(val);
    EXPECT_EQ(0, zmq_getsockopt(sp, ZMQ_TYPE, &val, &val_size));

    pool.remove(endpoint);

    // Socket must have been closed.
    val_size = sizeof(val);
    EXPECT_EQ(-1, zmq_getsockopt(sp, ZMQ_TYPE, &val, &val_size));

    // Add a socket that will be reaped.
    socket = make_shared<zmqpp::socket>(context, zmqpp::socket_type::push);
    socket->connect(endpoint);
    pool.register_socket(endpoint, socket, true);
    sp = static_cast<void*>(*socket);
    socket = nullptr;

    // Socket must be alive.
    val_size = sizeof(val);
    EXPECT_EQ(0, zmq_getsockopt(sp, ZMQ_TYPE, &val, &val_size));

    this_thread::sleep_for(chrono::seconds(2));

    // Socket must have been closed by reaper.
    val_size = sizeof(val);
    EXPECT_EQ(-1, zmq_getsockopt(sp, ZMQ_TYPE, &val, &val_size));
}
