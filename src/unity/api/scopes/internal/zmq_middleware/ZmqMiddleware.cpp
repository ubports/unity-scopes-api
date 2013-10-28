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

#include <unity/api/scopes/internal/zmq_middleware/ZmqMiddleware.h>

#include <unity/api/scopes/internal/zmq_middleware/ConnectionPool.h>
#include <unity/api/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/api/scopes/internal/zmq_middleware/QueryI.h>
#include <unity/api/scopes/internal/zmq_middleware/QueryCtrlI.h>
#include <unity/api/scopes/internal/zmq_middleware/RegistryI.h>
#include <unity/api/scopes/internal/zmq_middleware/ReplyI.h>
#include <unity/api/scopes/internal/zmq_middleware/ScopeI.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqQuery.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqRegistry.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/api/scopes/internal/zmq_middleware/RethrowException.h>
#include <unity/api/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;

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

namespace
{

char const* query_suffix = "-query";  // Appended to server_name_ to create query adapter name
char const* ctrl_suffix = "-ctrl";    // Appended to server_name_ to create control adapter name
char const* reply_suffix = "-reply";  // Appended to server_name_ to create reply adapter name

} // namespace

ZmqMiddleware::ZmqMiddleware(string const& server_name, string const& configfile, RuntimeImpl* runtime)
try :
    MiddlewareBase(runtime),
    server_name_(server_name),
    state_(Stopped)
{
    assert(!server_name.empty());

    // TODO: read config from file (thread pool size, invocation timeout, etc.
}
catch (zmqpp::exception const& e)
{
    rethrow_zmq_ex(e);
}

ZmqMiddleware::~ZmqMiddleware() noexcept
{
    try
    {
        stop();
        wait_for_shutdown();
    }
    catch (zmqpp::exception const& e)
    {
        // TODO: log exception
    }
    catch (...)
    {
        // TODO: log exception
    }
}

void ZmqMiddleware::start()
{
    unique_lock<mutex> lock(mutex_);

    switch (state_)
    {
        case Started:
        case Starting:
        {
            return; // Already started, or about to start, no-op
        }
        case Stopped:
        {
            // TODO: get directory from config
            // TODO: get pool size from config
            invokers_.reset(new ThreadPool(1));
            state_ = Started;
            state_changed_.notify_all();
            break;
        }
        default:
        {
            assert(false);
        }
    }
}

void ZmqMiddleware::stop()
{
    unique_lock<mutex> lock(mutex_);
    switch (state_)
    {
        case Stopped:
        {
            break;  // Already stopped, or about to stop, no-op
        }
        case Starting:
        {
            // Wait until start in progress has completed before stopping
            // Coverage excluded here because the window for which we are in this state is too
            // small to hit with a test.
            state_changed_.wait(lock, [this] { return state_ == Started; }); // LCOV_EXCL_LINE
            // FALLTHROUGH
        }
        case Started:
        {
            invokers_.reset();          // No more outgoing invocations
            for (auto& pair : am_)
            {
                pair.second->shutdown();
            }
            for (auto& pair : am_)
            {
                pair.second->wait_for_shutdown();
            }
            state_ = Stopped;
            state_changed_.notify_all();
            break;
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

void ZmqMiddleware::wait_for_shutdown()
{
    unique_lock<mutex> lock(mutex_);
    state_changed_.wait(lock, [this] { return state_ == Stopped; }); // LCOV_EXCL_LINE
}

MWRegistryProxy ZmqMiddleware::create_registry_proxy(string const& identity, string const& endpoint)
{
    MWRegistryProxy proxy;
    try
    {
        proxy.reset(new ZmqRegistry(this, endpoint, identity));
    }
    catch (zmqpp::exception const& e)
    {
        rethrow_zmq_ex(e);
    }
    return proxy;
}

MWScopeProxy ZmqMiddleware::create_scope_proxy(string const& identity, string const& endpoint)
{
    MWScopeProxy proxy;
    try
    {
        proxy.reset(new ZmqScope(this, endpoint, identity));
    }
    catch (zmqpp::exception const& e)
    {
        rethrow_zmq_ex(e);
    }
    return proxy;
}

MWQueryCtrlProxy ZmqMiddleware::add_query_ctrl_object(QueryCtrlObject::SPtr const& ctrl)
{
    assert(ctrl);

    MWQueryCtrlProxy proxy;
    try
    {
        shared_ptr<QueryCtrlI> qci(make_shared<QueryCtrlI>(ctrl));
        auto adapter = find_adapter(server_name_ + ctrl_suffix);
        function<void()> df;
        auto proxy = safe_add(df, adapter, "", qci);
        ctrl->set_disconnect_function(df);
        return ZmqQueryCtrlProxy(new ZmqQueryCtrl(this, proxy->endpoint(), proxy->identity()));
    }
    catch (...)
    {
        throw; // TODO
    }
    return proxy;
}

MWQueryProxy ZmqMiddleware::add_query_object(QueryObject::SPtr const& query)
{
    assert(query);

    MWQueryProxy proxy;
    try
    {
        shared_ptr<QueryI> qi(make_shared<QueryI>(query));
        auto adapter = find_adapter(server_name_ + query_suffix);
        function<void()> df;
        auto proxy = safe_add(df, adapter, "", qi);
        query->set_disconnect_function(df);
        return ZmqQueryProxy(new ZmqQuery(this, proxy->endpoint(), proxy->identity()));
    }
    catch (...)
    {
        throw; // TODO
    }
    return proxy;
}


MWRegistryProxy ZmqMiddleware::add_registry_object(string const& identity, RegistryObject::SPtr const& registry)
{
    assert(!identity.empty());
    assert(registry);

    MWRegistryProxy proxy;
    try
    {
        shared_ptr<RegistryI> ri(make_shared<RegistryI>(registry));
        auto adapter = find_adapter(server_name_);
        function<void()> df;
        auto proxy = safe_add(df, adapter, identity, ri);
        registry->set_disconnect_function(df);
        return ZmqRegistryProxy(new ZmqRegistry(this, proxy->endpoint(), proxy->identity()));
    }
    catch (...)
    {
        throw; // TODO
    }
    return proxy;
}

MWReplyProxy ZmqMiddleware::add_reply_object(ReplyObject::SPtr const& reply)
{
    assert(reply);

    MWReplyProxy proxy;
    try
    {
        shared_ptr<ReplyI> ri(make_shared<ReplyI>(reply));
        auto adapter = find_adapter(server_name_ + reply_suffix);
        function<void()> df;
        auto proxy = safe_add(df, adapter, "", ri);
        reply->set_disconnect_function(df);
        return ZmqReplyProxy(new ZmqReply(this, proxy->endpoint(), proxy->identity()));
    }
    catch (...)
    {
        throw; // TODO
    }
    return proxy;
}

MWScopeProxy ZmqMiddleware::add_scope_object(string const& identity, ScopeObject::SPtr const& scope)
{
    assert(!identity.empty());
    assert(scope);

    MWScopeProxy proxy;
    try
    {
        shared_ptr<ScopeI> si(make_shared<ScopeI>(scope));
        auto adapter = find_adapter(server_name_);
        function<void()> df;
        auto proxy = safe_add(df, adapter, identity, si);
        scope->set_disconnect_function(df);
        return ZmqScopeProxy(new ZmqScope(this, proxy->endpoint(), proxy->identity()));
    }
    catch (...)
    {
        throw; // TODO
    }
    return proxy;
}

zmqpp::context* ZmqMiddleware::context() const noexcept
{
    return const_cast<zmqpp::context*>(&context_);
}

ThreadPool* ZmqMiddleware::invoke_pool() const noexcept
{
    return invokers_.get();
}

namespace
{

bool has_suffix(string const& s, string const& suffix)
{
    auto s_len = s.length();
    auto suffix_len = suffix.length();
    if (s_len >= suffix_len)
    {
        return s.compare(s_len - suffix_len, suffix_len, suffix) == 0;
    }
    return false;
}

} // namespace

shared_ptr<ObjectAdapter> ZmqMiddleware::find_adapter(string const& name)
{
    lock_guard<mutex> lock(mutex_);

    auto it = am_.find(name);
    if (it != am_.end())
    {
        return it->second;
    }

    // We don't have the requested adapter yet, so we create it on the fly.
    int pool_size;
    RequestType type;
    if (has_suffix(name, query_suffix))
    {
        // The query adapter is single or multi-thread and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        type = RequestType::Oneway;
    }
    else if (has_suffix(name, ctrl_suffix))
    {
        // The ctrl adapter is single-threaded and supports oneway operations only.
        pool_size = 1;
        type = RequestType::Oneway;
    }
    else if (has_suffix(name, reply_suffix))
    {
        // The reply adapter is single- or multi-threaded and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        type = RequestType::Oneway;
    }
    else
    {
        // The normal adapter is single- or multi-threaded and supports twoway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        type = RequestType::Twoway;
    }
    // TODO: get directory of adapter from config
    // The query adapter is always inproc.
    string endpoint = (has_suffix(name, query_suffix) ? "inproc://" : "ipc://") + name;

    shared_ptr<ObjectAdapter> a(new ObjectAdapter(*this, name, endpoint, type, pool_size));
    a->activate();
    am_[name] = a;
    return a;
}

ZmqProxy ZmqMiddleware::safe_add(function<void()>& disconnect_func,
                                 shared_ptr<ObjectAdapter> const& adapter,
                                 string const& identity,
                                 shared_ptr<ServantBase> const& servant)
{
    string id = identity.empty() ? unique_id_.gen() : identity;

#if 0 // TODO: is this necessary? Needs to match what happens in stop()
    lock_guard<mutex> lock(mutex_);
    if (!adapter)
    {
        throw LogicException("Cannot add object to stopped middleware");
    }
#endif
    disconnect_func = [adapter, id]
    {
        try
        {
            adapter->remove(id);
        }
        catch (...)
        {
        }
    };
    return adapter->add(id, servant);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
