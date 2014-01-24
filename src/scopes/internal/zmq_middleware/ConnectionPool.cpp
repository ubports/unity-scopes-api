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

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

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
    zmqpp::socket_type stype = m == RequestMode::Twoway ? zmqpp::socket_type::request : zmqpp::socket_type::push;
    zmqpp::socket s(context_, stype);
    s.set(zmqpp::socket_option::linger, 0);
    s.connect(endpoint);
    return pool_.emplace(endpoint, Connection { move(s), m }).first->second.socket;
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
    pool_.emplace(endpoint, Connection { move(socket), m });
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity