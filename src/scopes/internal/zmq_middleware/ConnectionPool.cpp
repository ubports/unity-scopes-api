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

ConnectionPool::ConnectionPool(zmqpp::context& context, int close_after_idle_seconds)
    : context_(context)
    , reaper_(Reaper::create(1, close_after_idle_seconds))
    , thread_id_(this_thread::get_id())
{
}

ConnectionPool::~ConnectionPool()
{
    assert([this]() -> bool { lock_guard<mutex> lock(mutex_); return this_thread::get_id() == thread_id_; }());
    reaper_ = nullptr;
    pool_.clear();
}

shared_ptr<zmqpp::socket> ConnectionPool::find(std::string const& endpoint)
{
    assert(!endpoint.empty());
    assert([this]() -> bool { lock_guard<mutex> lock(mutex_); return this_thread::get_id() == thread_id_; }());

    lock_guard<mutex> lock(mutex_);       // Prevent race with reaper thread.

    // Look for existing connection.
    auto it = pool_.find(endpoint);
    if (it != pool_.end())
    {
        if (it->second.reap_item)
        {
            it->second.reap_item->refresh();  // Tell reaper that the entry was touched.
        }
        return it->second.socket;
    }

    // No existing connection yet, establish one.
    auto s = create_connection(endpoint);

    // Make a reap entry that closes this socket once expired.
    auto ri = reaper_->add([this, endpoint]{ remove(endpoint); });
    PoolEntry entry{endpoint, s, ri};
    pool_.emplace(make_pair(endpoint, entry));
    return s;
}

shared_ptr<zmqpp::socket> ConnectionPool::create_connection(std::string const& endpoint)
{
    assert(!mutex_.try_lock());  // Must be called with mutex_ locked.

    shared_ptr<zmqpp::socket> s = make_shared<zmqpp::socket>(context_, zmqpp::socket_type::push);
    // Allow short linger time so messages written just before we shut down
    // have some chance of being sent, and we don't block indefinitely if the
    // peer has gone away.
    s->set(zmqpp::socket_option::linger, 50);
    // We set a reconnect interval of 20 ms, so we get to the peer quickly, in case
    // the peer hasn't finished binding to its endpoint yet after the first query
    // is sent. We back off exponentially to one second.
    // The reaper removes the entry once it has been idle for close_after_idle_seconds.
    // This stops Zmq from trying to indefinitely re-establish a connection to a peer that,
    // potentially, may never come back.
    s->set(zmqpp::socket_option::reconnect_interval, 20);
    s->set(zmqpp::socket_option::reconnect_interval_max, 1000);
    s->connect(endpoint);
    return s;
}

void ConnectionPool::remove(std::string const& endpoint)
{
    assert(!endpoint.empty());

    lock_guard<mutex> lock(mutex_);  // Prevent race with reaper thread.

    auto const& it = pool_.find(endpoint);
    if (it != pool_.end())
    {
        pool_.erase(it);
    }
}

void ConnectionPool::register_socket(std::string const& endpoint,
                                     shared_ptr<zmqpp::socket> const& socket,
                                     bool idle_timeout)
{
    assert(!endpoint.empty());
    assert([this]() -> bool { lock_guard<mutex> lock(mutex_); return this_thread::get_id() == thread_id_; }());

    lock_guard<mutex> lock(mutex_);       // Prevent race with reaper thread.

    auto ri = idle_timeout ? reaper_->add([this, endpoint]{ remove(endpoint); }) : nullptr;
    PoolEntry entry{endpoint, socket, ri};
    pool_.emplace(endpoint, entry);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
