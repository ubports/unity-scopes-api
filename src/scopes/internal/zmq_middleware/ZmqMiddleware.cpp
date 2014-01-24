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

#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/zmq_middleware/ConnectionPool.h>
#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/internal/zmq_middleware/QueryI.h>
#include <unity/scopes/internal/zmq_middleware/QueryCtrlI.h>
#include <unity/scopes/internal/zmq_middleware/RegistryI.h>
#include <unity/scopes/internal/zmq_middleware/ReplyI.h>
#include <unity/scopes/internal/zmq_middleware/ScopeI.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQuery.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqRegistry.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/scopes/internal/zmq_middleware/RethrowException.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <iostream>  // TODO: remove this once logging is added

using namespace std;

namespace unity
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
    state_(Stopped),
    config_(configfile),
    twoway_timeout_(300),  // TODO: get timeout from config
    locate_timeout_(1500)  // TODO: get timeout from config
{
    assert(!server_name.empty());
}
catch (zmqpp::exception const& e)
{
    rethrow_zmq_ex(e);
}

ZmqMiddleware::~ZmqMiddleware()
{
    try
    {
        stop();
        wait_for_shutdown();
    }
    catch (std::exception const& e)
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
    unique_lock<mutex> lock(state_mutex_);

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
            {
                lock_guard<mutex> lock(data_mutex_);
                invokers_.reset(new ThreadPool(1));
            }
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
    unique_lock<mutex> lock(state_mutex_);
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
            {
                lock_guard<mutex> lock(data_mutex_);
                // No more outgoing invocations
                invokers_.reset();
            }
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
    unique_lock<mutex> lock(state_mutex_);
    state_changed_.wait(lock, [this] { return state_ == Stopped; }); // LCOV_EXCL_LINE
}

MWRegistryProxy ZmqMiddleware::create_registry_proxy(string const& identity, string const& endpoint)
{
    MWRegistryProxy proxy;
    try
    {
        proxy.reset(new ZmqRegistry(this, endpoint, identity, "Registry", twoway_timeout_));
    }
    catch (zmqpp::exception const& e)
    {
        rethrow_zmq_ex(e);
    }
    return proxy;
}

MWScopeProxy ZmqMiddleware::create_scope_proxy(string const& identity)
{
    MWScopeProxy proxy;
    try
    {
        string endpoint = "ipc://" + config_.private_dir() + "/" + identity;
        proxy.reset(new ZmqScope(this, endpoint, identity, "Scope", twoway_timeout_));
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
        proxy.reset(new ZmqScope(this, endpoint, identity, "Scope", twoway_timeout_));
    }
    catch (zmqpp::exception const& e)
    {
        rethrow_zmq_ex(e);
    }
    return proxy;
}

MWQueryCtrlProxy ZmqMiddleware::add_query_ctrl_object(QueryCtrlObjectBase::SPtr const& ctrl)
{
    assert(ctrl);

    MWQueryCtrlProxy proxy;
    try
    {
        shared_ptr<QueryCtrlI> qci(make_shared<QueryCtrlI>(ctrl));
        auto adapter = find_adapter(server_name_ + ctrl_suffix, config_.private_dir());
        function<void()> df;
        auto proxy = safe_add(df, adapter, "", qci);
        ctrl->set_disconnect_function(df);
        return ZmqQueryCtrlProxy(new ZmqQueryCtrl(this, proxy->endpoint(), proxy->identity(), "QueryCtrl"));
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_query_ctrl_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}

void ZmqMiddleware::add_dflt_query_ctrl_object(QueryCtrlObjectBase::SPtr const& ctrl)
{
    assert(ctrl);

    MWQueryCtrlProxy proxy;
    try
    {
        shared_ptr<QueryCtrlI> qci(make_shared<QueryCtrlI>(ctrl));
        auto adapter = find_adapter(server_name_ + ctrl_suffix, config_.private_dir());
        auto df = safe_dflt_add(adapter, "QueryCtrl", qci);
        ctrl->set_disconnect_function(df);
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_dflt_query_ctrl_object(): " << e.what() << endl;
        throw;
    }
}

MWQueryProxy ZmqMiddleware::add_query_object(QueryObjectBase::SPtr const& query)
{
    assert(query);

    MWQueryProxy proxy;
    try
    {
        shared_ptr<QueryI> qi(make_shared<QueryI>(query));
        auto adapter = find_adapter(server_name_ + query_suffix, config_.private_dir());
        function<void()> df;
        auto proxy = safe_add(df, adapter, "", qi);
        query->set_disconnect_function(df);
        return ZmqQueryProxy(new ZmqQuery(this, proxy->endpoint(), proxy->identity(), "Query"));
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_query_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}


MWRegistryProxy ZmqMiddleware::add_registry_object(string const& identity, RegistryObjectBase::SPtr const& registry)
{
    assert(!identity.empty());
    assert(registry);

    MWRegistryProxy proxy;
    try
    {
        shared_ptr<RegistryI> ri(make_shared<RegistryI>(registry));
        auto adapter = find_adapter(server_name_, runtime()->registry_endpointdir());
        function<void()> df;
        auto proxy = safe_add(df, adapter, identity, ri);
        registry->set_disconnect_function(df);
        return ZmqRegistryProxy(new ZmqRegistry(this, proxy->endpoint(), proxy->identity(), "Registry", twoway_timeout_));
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_registry_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}

MWReplyProxy ZmqMiddleware::add_reply_object(ReplyObjectBase::SPtr const& reply)
{
    assert(reply);

    MWReplyProxy proxy;
    try
    {
        shared_ptr<ReplyI> ri(make_shared<ReplyI>(reply));
        auto adapter = find_adapter(server_name_ + reply_suffix, config_.public_dir());
        function<void()> df;
        auto proxy = safe_add(df, adapter, "", ri);
        reply->set_disconnect_function(df);
        return ZmqReplyProxy(new ZmqReply(this, proxy->endpoint(), proxy->identity(), "Reply"));
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_reply_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}

MWScopeProxy ZmqMiddleware::add_scope_object(string const& identity, ScopeObjectBase::SPtr const& scope)
{
    assert(!identity.empty());
    assert(scope);

    MWScopeProxy proxy;
    try
    {
        shared_ptr<ScopeI> si(make_shared<ScopeI>(scope));
        auto adapter = find_adapter(server_name_, config_.private_dir());
        function<void()> df;
        auto proxy = safe_add(df, adapter, identity, si);
        scope->set_disconnect_function(df);
        return ZmqScopeProxy(new ZmqScope(this, proxy->endpoint(), proxy->identity(), "Scope", twoway_timeout_));
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_scope_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}

void ZmqMiddleware::add_dflt_scope_object(ScopeObjectBase::SPtr const& scope)
{
    assert(scope);

    MWScopeProxy proxy;
    try
    {
        shared_ptr<ScopeI> si(make_shared<ScopeI>(scope));
        auto adapter = find_adapter(server_name_, config_.private_dir());
        auto df = safe_dflt_add(adapter, "Scope", si);
        scope->set_disconnect_function(df);
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_dflt_scope_object(): " << e.what() << endl;
        throw;
    }
}

zmqpp::context* ZmqMiddleware::context() const noexcept
{
    return const_cast<zmqpp::context*>(&context_);
}

ThreadPool* ZmqMiddleware::invoke_pool()
{
    lock(state_mutex_, data_mutex_);
    unique_lock<mutex> state_lock(state_mutex_, std::adopt_lock);
    lock_guard<mutex> invokers_lock(data_mutex_, std::adopt_lock);
    if (state_ == Starting)
    {
        state_changed_.wait(state_lock, [this] { return state_ != Starting; }); // LCOV_EXCL_LINE
    }
    if (state_ == Stopped)
    {
        throw MiddlewareException("Cannot invoke operations while middleware is stopped");
    }
    return invokers_.get();
}

int64_t ZmqMiddleware::locate_timeout() const noexcept
{
    return locate_timeout_;
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

shared_ptr<ObjectAdapter> ZmqMiddleware::find_adapter(string const& name, string const& endpoint_dir)
{
    lock_guard<mutex> lock(data_mutex_);

    auto it = am_.find(name);
    if (it != am_.end())
    {
        return it->second;
    }

    // We don't have the requested adapter yet, so we create it on the fly.
    int pool_size;
    RequestMode mode;
    if (has_suffix(name, query_suffix))
    {
        // The query adapter is single or multi-threaded and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        mode = RequestMode::Oneway;
    }
    else if (has_suffix(name, ctrl_suffix))
    {
        // The ctrl adapter is single-threaded and supports oneway operations only.
        pool_size = 1;
        mode = RequestMode::Oneway;
    }
    else if (has_suffix(name, reply_suffix))
    {
        // The reply adapter is single- or multi-threaded and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        mode = RequestMode::Oneway;
    }
    else
    {
        // The normal adapter is single- or multi-threaded and supports twoway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        mode = RequestMode::Twoway;
    }

    // The query adapter is always inproc.
    string endpoint;
    if (has_suffix(name, query_suffix))
    {
        endpoint = "inproc://" + name;
    }
    else
    {
        endpoint = "ipc://" + endpoint_dir + "/" + name;
    }

    shared_ptr<ObjectAdapter> a(new ObjectAdapter(*this, name, endpoint, mode, pool_size));
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

    disconnect_func = [adapter, id]
    {
        try
        {
            adapter->remove(id);
        }
        catch (...)
        {
            // No error here; for concurrent invocations that each disconnect the servant,
            // only one of them will succeed.
        }
    };
    return adapter->add(id, servant);
}

function<void()> ZmqMiddleware::safe_dflt_add(shared_ptr<ObjectAdapter> const& adapter,
                                              string const& category,
                                              shared_ptr<ServantBase> const& servant)
{
    function<void()> disconnect_func = [adapter, category]
    {
        try
        {
            adapter->remove_dflt_servant(category);
        }
        catch (...)
        {
            // No error here; for concurrent invocations that each disconnect the servant,
            // only one of them will succeed.
        }
    };
    adapter->add_dflt_servant(category, servant);
    return disconnect_func;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity