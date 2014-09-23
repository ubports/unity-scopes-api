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

ConnectionPool::ConnectionPool(zmqpp::context& context)
    : context_(context)
{
}

ConnectionPool::~ConnectionPool()
{
    pool_.clear();
}

zmqpp::socket& ConnectionPool::find(std::string const& endpoint)
{
    assert(!endpoint.empty());

    // Look for existing connection.
    auto const& it = pool_.find(endpoint);
    if (it != pool_.end())
    {
        return it->second;
    }

    // No existing connection yet, establish one.
    auto s = create_connection(endpoint);
    return pool_.emplace(make_pair(endpoint, move(s))).first->second;
}

zmqpp::socket ConnectionPool::create_connection(std::string const& endpoint)
{
    zmqpp::socket s(context_, zmqpp::socket_type::push);
    // Allow short linger time so messages written just before we shut down
    // have some chance of being sent, and we don't block indefinitely if the
    // peer has gone away.
    s.set(zmqpp::socket_option::linger, 50);
    s.connect(endpoint);
    return move(s);
}

void ConnectionPool::remove(std::string const& endpoint)
{
    assert(!endpoint.empty());

    auto const& it = pool_.find(endpoint);
    if (it != pool_.end())
    {
        pool_.erase(it);
    }
}

void ConnectionPool::register_socket(std::string const& endpoint, zmqpp::socket socket)
{
    assert(!endpoint.empty());

    pool_.emplace(endpoint, move(socket));
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
