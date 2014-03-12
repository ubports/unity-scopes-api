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

#include <unity/scopes/internal/zmq_middleware/ConnectionPool.h>

#include <unity/scopes/ScopeExceptions.h>

#include <cassert>
#include <iostream> // TODO: remove this

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

// Take ownership of the passed pair <endpoint, SocketData>.

Socket::Socket(ConnectionPool* pool, pool_private::CPool::value_type pool_entry)
    : pool_(pool)
    , pool_entry_(move(pool_entry))
    , removed_(false)
{
}

// Return ownership of the entry back to the pool if remove() wasn't called earlier.
// If removed was called, the pool entry goes out of scope and closes the socket.

Socket::~Socket()
{
    if (!removed_)
    {
        pool_->put(move(pool_entry_));
    }
}

zmqpp::socket& Socket::zmqpp_socket()
{
    return pool_entry_.second.socket;
}

void Socket::remove()
{
cerr << "Remove " << pool_entry_.first << endl;
    removed_ = true;
}

ConnectionPool::ConnectionPool(zmqpp::context& context) :
    context_(context)
{
}

ConnectionPool::~ConnectionPool()
{
    pool_.clear();
}

zmqpp::socket& ConnectionPool::find(std::string const& endpoint, RequestMode m)
{
    assert(!endpoint.empty());

    lock_guard<mutex> lock(mutex_);

    // Look for existing connection. If there is one, but with the wrong type, we throw.
    auto const& it = pool_.find(endpoint);
    if (it != pool_.end())
    {
        if (it->second.mode != m)
        {
            string msg("ConnectionPool::find(): cannot send " + to_string(m) +
                       " request via " + to_string(it->second.mode) + " connection (endpoint: " + endpoint + ")");
            throw MiddlewareException(msg);
        }
        return it->second.socket;
    }

    // No existing connection yet, establish one.
    auto entry = create_connection(endpoint, m, -1);
    return pool_.emplace(move(entry))->second.socket;
}

void ConnectionPool::remove(std::string const& endpoint)
{
    assert(!endpoint.empty());

    lock_guard<mutex> lock(mutex_);

    auto const& it = pool_.find(endpoint);
    if (it != pool_.end())
    {
        pool_.erase(it);
    }
}

void ConnectionPool::register_socket(std::string const& endpoint, zmqpp::socket socket, RequestMode m)
{
    assert(!endpoint.empty());

    lock_guard<mutex> lock(mutex_);
    pool_.emplace(endpoint, pool_private::SocketData{ move(socket), m });
}

Socket ConnectionPool::take(string const& endpoint, int64_t timeout)
{
    lock_guard<mutex> lock(mutex_);

    auto it = pool_.find(endpoint);
    if (it != pool_.end())
    {
        if (it->second.mode != RequestMode::Twoway)
        {
            string msg("ConnectionPool::take(): cannot send " + to_string(RequestMode::Twoway) +
                       " request via " + to_string(it->second.mode) + " connection (endpoint: " + endpoint + ")");
            throw MiddlewareException(msg);
        }
        cerr << "take: returning existing socket for " << endpoint << endl;
        Socket s(this, move(*it));
        pool_.erase(it);
        return s;
    }
    cerr << "take: returning new socket for " << endpoint << endl;
    return Socket(this, create_connection(endpoint, RequestMode::Twoway, timeout));
}

void ConnectionPool::put(pool_private::CPool::value_type entry)
{
    lock_guard<mutex> lock(mutex_);
    pool_.emplace(move(entry));
}

pool_private::CPool::value_type ConnectionPool::create_connection(std::string const& endpoint,
                                                                  RequestMode m,
                                                                  int64_t timeout)
{
    zmqpp::socket_type stype = m == RequestMode::Twoway ? zmqpp::socket_type::request : zmqpp::socket_type::push;
    zmqpp::socket s(context_, stype);
    auto zmqpp_timeout = m == RequestMode::Twoway ? int32_t(timeout) : 0;
    s.set(zmqpp::socket_option::linger, zmqpp_timeout);
    cerr << "connecting " << to_string(m) << " to " << endpoint << " (" << zmqpp_timeout << ")" << endl;
    s.connect(endpoint);
    return pool_private::CPool::value_type{ endpoint, pool_private::SocketData{ move(s), m } };
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
