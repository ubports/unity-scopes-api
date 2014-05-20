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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_OBJECTADAPTER_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_OBJECTADAPTER_H

#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>

#include <unity/scopes/ScopeExceptions.h>
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

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ServantBase;
class StopPublisher;
class ZmqMiddleware;

class ObjectAdapter final
{
public:
    NONCOPYABLE(ObjectAdapter);

    ObjectAdapter(ZmqMiddleware& mw,
                  std::string const& name,
                  std::string const& endpoint,
                  RequestMode m,
                  int pool_size,
                  int64_t idle_timeout = 0);
    ~ObjectAdapter();

    ZmqMiddleware* mw() const;
    std::string name() const;
    std::string endpoint() const;

    ZmqProxy add(std::string const& id, std::shared_ptr<ServantBase> const& obj);
    void remove(std::string const& id);
    std::shared_ptr<ServantBase> find(std::string const& id) const;

    void add_dflt_servant(std::string const& category, std::shared_ptr<ServantBase> const& obj);
    void remove_dflt_servant(std::string const& category);
    std::shared_ptr<ServantBase> find_dflt_servant(std::string const& id) const;

    void activate();
    void shutdown();
    void wait_for_shutdown();

private:
    // An adapter transitions through these states in order:
    //
    // Inactive -> Activating -> Active -> Deactivating -> Destroyed.
    //
    // If the adapter is destroyed without ever having been activated, the transitions are
    //
    // Inactive -> Deactivating -> Destroyed.
    //
    // A deactivated adapter cannot be reactivated.
    // The Failed state is reachable from any of the other states and indicates
    // a fatal error condition.
    enum AdapterState { Inactive, Activating, Active, Deactivating, Destroyed, Failed };
    void throw_bad_state(std::string const& label, AdapterState state) const;

    void run_workers();
    // void init_ctrl_socket();
    // zmqpp::socket subscribe_to_ctrl_socket();
    // void stop_workers() noexcept;

    std::shared_ptr<ServantBase> find_servant(std::string const& id, std::string const& category);

    // Thread start functions
    void broker_thread();
    void worker_thread();

    void cleanup();
    void join_with_all_threads();

    void store_exception(scopes::MiddlewareException& ex);

    ZmqMiddleware& mw_;
    std::string name_;
    std::string endpoint_;
    RequestMode mode_;
    int pool_size_;
    int64_t idle_timeout_;
    // std::unique_ptr<zmqpp::socket> ctrl_;       // PUB socket to signal when to deactivate
    // std::mutex ctrl_mutex_;                     // Synchronizes access to ctrl_ when sending
    std::unique_ptr<StopPublisher> stopper_;    // Used to signal threads when it's time to terminate
    std::thread broker_;                        // Connects router with dealer
    std::vector<std::thread> workers_;          // Threads for incoming invocations
    std::atomic_int num_workers_;               // For handshake with parent
    std::promise<void> ready_;                  // For handshake with child threads
    std::mutex ready_mutex_;                    // Protects ready_
    std::once_flag once_;
    std::exception_ptr exception_;              // Failed threads deposit their exception here

    AdapterState state_;
    std::condition_variable state_changed_;
    mutable std::mutex state_mutex_;

    // Map of object identity and servant pairs
    typedef std::unordered_map<std::string, std::shared_ptr<ServantBase>> ServantMap;
    ServantMap servants_;
    ServantMap dflt_servants_;
    mutable std::mutex map_mutex_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
