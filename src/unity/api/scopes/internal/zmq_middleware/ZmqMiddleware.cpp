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

#include <unity/api/scopes/internal/zmq_middleware/ObjectAdapter.h>
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

char const* ctrl_suffix = "-ctrl";    // Appended to server_name_ to create control adapter name
char const* reply_suffix = "-reply";  // Appended to server_name_ to create reply adapter name

} // namespace

ZmqMiddleware::ZmqMiddleware(string const& server_name, string const& configfile, RuntimeImpl* runtime) :
    MiddlewareBase(runtime),
    server_name_(server_name)
{
    // TODO: get directory from config
    assert(!server_name.empty());
    try
    {
#if 0
        Ice::PropertiesPtr props = Ice::createProperties();
        props->load(configfile);


        // Set properties for the object adapters. We need to do this before we create the communicator
        // because Ice reads properties only once, during communicator creation.

        // TODO: dubious, because -f path may not be last in the endpoint config need to parse out the endpoint
        // or maybe get it from the adapter.
        props->setProperty(server_name_ + ctrl_suffix + ".Endpoints",  props->getProperty(server_name_ + ".Endpoints") + ctrl_suffix);
        props->setProperty(server_name_ + reply_suffix + ".Endpoints",  props->getProperty(server_name_ + ".Endpoints") + reply_suffix);

        // The -ctrl adapter gets its own thread pool with a single thread.
        props->setProperty(server_name_ + ctrl_suffix + ".ThreadPool.Size", "1");

        // TODO: configurable pool size for normal and reply adapter

        Ice::InitializationData init_data;
        init_data.properties = props;
        ic_ = Ice::initialize(init_data);
#endif
    }
    catch (zmqpp::exception const& e)
    {
        rethrow_zmq_ex(e);
    }
}

ZmqMiddleware::~ZmqMiddleware() noexcept
{
    try
    {
#if 0
        stop();
        ic_->destroy();
#endif
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
}

void ZmqMiddleware::stop()
{
#if 0
    try
    {
        // TODO: need to cancel any outstanding stuff here?
        // TODO: need to disable proxy?

        AdapterMap am;
        {
            lock_guard<mutex> lock(mutex_);
            am = am_;
            am_.clear();
        }
        for (auto a : am)
        {
            a.second->deactivate();
            a.second->waitForDeactivate();
        }
        am.clear();
    }
    catch (Zmq::exception const& e)
    {
        // TODO: log exception
        rethrow_zmq_ex(e);
    }
#endif
}

void ZmqMiddleware::wait_for_shutdown()
{
#if 0
    try
    {
        ic_->waitForShutdown();
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
#endif
}

MWRegistryProxy ZmqMiddleware::create_registry_proxy(string const& identity, string const& endpoint)
{
    try
    {
    }
    catch (zmqpp::exception const& e)
    {
        rethrow_zmq_ex(e);
    }
#if 0
    MWRegistryProxy proxy;
    try
    {
        string proxy_string = identity + ":" + endpoint;
        middleware::RegistryPrx r = middleware::RegistryPrx::uncheckedCast(ic_->stringToProxy(proxy_string));
        if (!r)
        {
            // TODO: throw config exception
            assert(false);
        }
        proxy.reset(new IceRegistry(this, r));
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
    return proxy;
#endif
}

MWScopeProxy ZmqMiddleware::create_scope_proxy(string const& identity, string const& endpoint)
{
#if 0
    MWScopeProxy proxy;
    try
    {
        string proxy_string = identity + ":" + endpoint;
        middleware::ScopePrx s = middleware::ScopePrx::uncheckedCast(ic_->stringToProxy(proxy_string));
        if (!s)
        {
            throw ConfigException("Ice: could not create proxy from string \"" + proxy_string + "\"");
        }
        proxy.reset(new IceScope(this, s));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
#endif
}

MWQueryCtrlProxy ZmqMiddleware::add_query_ctrl_object(QueryCtrlObject::SPtr const& ctrl)
{
#if 0
    assert(ctrl);

    MWQueryCtrlProxy proxy;
    try
    {
        QueryCtrlIPtr ci = new QueryCtrlI(ctrl);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_ + ctrl_suffix);
        function<void()> df;
        proxy.reset(new IceQueryCtrl(this, middleware::QueryCtrlPrx::uncheckedCast(safe_add(df, a, ci))));
        ctrl->set_disconnect_function(df);
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
#endif
}

MWQueryProxy ZmqMiddleware::add_query_object(QueryObject::SPtr const& query)
{
#if 0
    assert(query);

    MWQueryProxy proxy;
    try
    {
        QueryIPtr qi = new QueryI(this, query);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_);
        function<void()> df;
        proxy.reset(new IceQuery(this, middleware::QueryPrx::uncheckedCast(safe_add(df, a, qi))));
        query->set_disconnect_function(df);
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
#endif
}


MWRegistryProxy ZmqMiddleware::add_registry_object(string const& identity, RegistryObject::SPtr const& registry)
{
#if 0
    assert(!identity.empty());
    assert(registry);

    MWRegistryProxy proxy;
    try
    {
        RegistryIPtr ri = new RegistryI(registry);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_);
        function<void()> df;
        proxy.reset(new IceRegistry(this, middleware::RegistryPrx::uncheckedCast(safe_add(df, a, ri, identity))));
        registry->set_disconnect_function(df);
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
#endif
}

MWReplyProxy ZmqMiddleware::add_reply_object(ReplyObject::SPtr const& reply)
{
#if 0
    assert(reply);

    MWReplyProxy proxy;
    try
    {
        ReplyIPtr si = new ReplyI(reply);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_ + reply_suffix);
        function<void()> df;
        proxy.reset(new IceReply(this, middleware::ReplyPrx::uncheckedCast(safe_add(df, a, si))));
        reply->set_disconnect_function(df);
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
#endif
}

MWScopeProxy ZmqMiddleware::add_scope_object(string const& identity, ScopeObject::SPtr const& scope)
{
#if 0
    assert(!identity.empty());
    assert(scope);

    MWScopeProxy proxy;
    try
    {
        ScopeIPtr si = new ScopeI(this, scope);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_ + ctrl_suffix);
        function<void()> df;
        proxy.reset(new IceScope(this, middleware::ScopePrx::uncheckedCast(safe_add(df, a, si, identity))));
        scope->set_disconnect_function(df);
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
#endif
}

zmqpp::context& ZmqMiddleware::context() noexcept
{
    return context_;
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
    ObjectAdapter::Type type;
    if (has_suffix(name, ctrl_suffix))
    {
        // The ctrl adapter is single-threaded and supports oneway operations only.
        pool_size = 1;
        type = ObjectAdapter::Type::Oneway;
    }
    else if (has_suffix(name, reply_suffix))
    {
        // The reply adapter is single- or multi-threaded and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        type = ObjectAdapter::Type::Oneway;
    }
    else
    {
        // The normal adapter is single- or multi-threaded and supports twoway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        type = ObjectAdapter::Type::Twoway;
        // TODO: get pool size from config
    }
    // TODO: get directory of adapter from config
    string endpoint = "ipc://" + name;

    shared_ptr<ObjectAdapter> a(new ObjectAdapter(*this, name, endpoint, pool_size, type));
    am_[name] = a;
    return a;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
