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

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_OBJECTADAPTER_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_OBJECTADAPTER_H

#include <unity/api/scopes/internal/zmq_middleware/ZmqObjectProxy.h>

#include <unity/util/NonCopyable.h>
#include <zmqpp/socket.hpp>

#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

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

class ServantBase;
class ZmqMiddleware;

class ObjectAdapter final : private util::NonCopyable
{
public:
    ObjectAdapter(ZmqMiddleware& mw,
                  std::string const& name,
                  std::string const& endpoint,
                  RequestType t,
                  int pool_size);
    ~ObjectAdapter() noexcept;

    ZmqMiddleware* mw() const;
    std::string name() const;
    std::string endpoint() const;

    ZmqProxy add(std::string const& id, std::shared_ptr<ServantBase> const& obj);
    void remove(std::string const& id);
    std::shared_ptr<ServantBase> find(std::string const& id) const noexcept;

    void activate() noexcept;
    void shutdown() noexcept;
    void wait_for_shutdown() noexcept;

private:
    void run_workers();

    // Thread start functions
    void broker_thread();
    void worker_thread();

    ZmqMiddleware& mw_;
    std::string name_;
    std::string endpoint_;
    RequestType type_;
    int pool_size_;
    std::unique_ptr<zmqpp::socket> frontend_;   // Connected to endpoint, visible to clients
    std::unique_ptr<zmqpp::socket> backend_;    // Workers read from this
    std::unique_ptr<zmqpp::socket> ctrl_;       // PUB socket to signal when to deactivate
    std::thread broker_;                        // Connects router with dealer
    std::vector<std::thread> workers_;          // Threads for incoming invocations
    std::atomic_int num_workers_;               // For handshake with parent
    std::promise<void> ready_;                  // For handshake with child threads
    std::mutex ready_mutex_;                    // Protects ready_

    enum AdapterState { Inactive, Activating, Active, ShuttingDown };
    AdapterState state_;
    std::condition_variable state_changed_;

    // Map of object identity and servant pairs
    typedef std::unordered_map<std::string, std::shared_ptr<ServantBase>> ServantMap;
    ServantMap servants_;

    mutable std::mutex mutex_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
