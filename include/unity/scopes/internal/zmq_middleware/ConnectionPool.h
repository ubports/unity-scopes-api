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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_CONNECTIONPOOL_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_CONNECTIONPOOL_H

#include <unity/scopes/internal/Reaper.h>
#include <unity/scopes/internal/zmq_middleware/RequestMode.h>
#include <unity/util/NonCopyable.h>

#include <zmqpp/socket.hpp>

#include <string>
#include <unordered_map>

// Simple connection pool for oneway invocations. Zmq sockets are not thread-safe, which means
// that a proxy cannot directly contain a socket because that would cause invocations on the same proxy by
// different threads to crash.
// So, we maintain a pool of invocation threads, with each thread keeping its own cache of sockets.
// Sockets are indexed by endpoint and created lazily.
// Any socket that has been idle for close_after_idle_seconds is removed from the pool by a reaper.
// This is to prevent Zmq from endlessly trying to reconnect to the peer.
//
// WARNING: A separate instance of the pool is required for each calling thread.
//          The code asserts if different threads call find() or if the thread that
//          destroys the pool is not the same thread as the one that created it.

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ConnectionPool final
{
public:
    NONCOPYABLE(ConnectionPool);
    ConnectionPool(zmqpp::context& context, int close_after_idle_seconds = 10);
    ~ConnectionPool();
    std::shared_ptr<zmqpp::socket> find(std::string const& endpoint);
    void remove(std::string const& endpoint);
    void register_socket(std::string const& endpoint,
                         std::shared_ptr<zmqpp::socket> const& socket,
                         bool idle_timeout = true);

private:
    struct PoolEntry
    {
        std::string endpoint;
        std::shared_ptr<zmqpp::socket> socket;
        ReapItem::SPtr reap_item;
    };
    typedef std::unordered_map<std::string, PoolEntry> CPool;

    std::shared_ptr<zmqpp::socket> create_connection(std::string const& endpoint);

    zmqpp::context& context_;
    CPool pool_;

    Reaper::SPtr reaper_;        // Removes connection from the pool after close_after_idle_seconds of idle time.
    std::mutex mutex_;
    std::thread::id thread_id_;  // For debug build, to assert that pool is used as thread_local static only.
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
