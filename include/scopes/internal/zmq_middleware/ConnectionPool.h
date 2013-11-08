/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_CONNECTIONPOOL_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_CONNECTIONPOOL_H

#include <scopes/internal/zmq_middleware/RequestType.h>
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

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ConnectionPool final : private util::NonCopyable
{
public:
    ConnectionPool(zmqpp::context& context);
    ~ConnectionPool();
    zmqpp::socket& find(std::string const& endpoint, RequestType t);

private:
    struct Connection
    {
        zmqpp::socket socket;
        RequestType type;
    };

    typedef std::unordered_map<std::string, Connection> CPool;

    zmqpp::context& context_;
    CPool pool_;
    std::mutex mutex_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
