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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_CONNECTIONPOOL_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_CONNECTIONPOOL_H

#include <unity/scopes/internal/zmq_middleware/RequestMode.h>
#include <unity/util/NonCopyable.h>

#include <zmqpp/socket.hpp>

#include <mutex>
#include <string>
#include <unordered_map>

// Simple connection pool for invocation threads. Zmq sockets are not thread-safe, which means
// that a proxy cannot directly contain a socket because that would cause invocations on the same proxy by
// different threads to crash.
// So, we maintain a pool of invocation threads, with each thread keeping its own cache of sockets.
// Sockets are indexed by adapter name and created lazily.

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

namespace pool_private
{

struct SocketData
{
    zmqpp::socket socket;
    RequestMode mode;
};

typedef std::unordered_map<std::string, pool_private::SocketData> CPool;

}

class ConnectionPool;

class Socket final
{
public:
    Socket(Socket&&);
    Socket& operator=(Socket&&);
    ~Socket();

    zmqpp::socket& zmqpp_socket();
    void invalidate();

private:
    Socket(ConnectionPool* pool, pool_private::CPool::value_type pool_entry);  // Only instantiated by ConnectionPool

    ConnectionPool* pool_;                        // Owning pool pointer
    pool_private::CPool::value_type pool_entry_;
    bool invalidated_;

    friend class ConnectionPool;
};

class ConnectionPool final
{
public:
    NONCOPYABLE(ConnectionPool);
    ConnectionPool(zmqpp::context& context);
    ~ConnectionPool();
    zmqpp::socket& find(std::string const& endpoint, RequestMode m);
    void remove(std::string const& endpoint);
    void register_socket(std::string const& endpoint, zmqpp::socket socket, RequestMode m);

    // Removes a twoway socket from the pool and returns it, creating the
    // connection if none can be found. This gives exclusive use of the socket
    // to an outgoing twoway request for the duration. The Socket destructor
    // puts the entry back into the pool by calling put().
    Socket take(std::string const& endpoint, int64_t timeout);

private:
    // Returns the passed entry back to the pool.
    void put(pool_private::CPool::value_type entry);

    pool_private::CPool::value_type create_connection(std::string const& endpoint, RequestMode m, int64_t timeout);

    zmqpp::context& context_;
    pool_private::CPool pool_;
    std::mutex mutex_;

    friend class Socket;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
