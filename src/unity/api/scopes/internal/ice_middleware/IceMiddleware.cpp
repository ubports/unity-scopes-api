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

#include <unity/api/scopes/internal/ice_middleware/IceMiddleware.h>

#include <unity/api/scopes/internal/ice_middleware/IceQueryCtrl.h>
#include <unity/api/scopes/internal/ice_middleware/IceQuery.h>
#include <unity/api/scopes/internal/ice_middleware/IceRegistry.h>
#include <unity/api/scopes/internal/ice_middleware/IceReply.h>
#include <unity/api/scopes/internal/ice_middleware/IceScope.h>
#include <unity/api/scopes/internal/ice_middleware/QueryCtrlI.h>
#include <unity/api/scopes/internal/ice_middleware/QueryI.h>
#include <unity/api/scopes/internal/ice_middleware/RegistryI.h>
#include <unity/api/scopes/internal/ice_middleware/ReplyI.h>
#include <unity/api/scopes/internal/ice_middleware/RethrowException.h>
#include <unity/api/scopes/internal/ice_middleware/ScopeI.h>
#include <unity/api/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <IceUtil/UUID.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

namespace
{

char const* ctrl_suffix = "-ctrl";    // Appended to server_name_ to create control adapter name
char const* reply_suffix = "-reply";  // Appended to server_name_ to create reply adapter name

} // namespace

IceMiddleware::IceMiddleware(string const& server_name, string const& configfile)
    : server_name_(server_name)
{
    assert(!server_name.empty());
    try
    {
        Ice::PropertiesPtr props = Ice::createProperties();
        props->load(configfile);

        // TODO: get directory from config

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
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
}

IceMiddleware::~IceMiddleware() noexcept
{
    try
    {
        stop();
        ic_->destroy();
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
    }
    catch (...)
    {
        // TODO: log exception
    }
}

void IceMiddleware::start()
{
}

void IceMiddleware::stop()
{
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
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
}

void IceMiddleware::wait_for_shutdown()
{
    try
    {
        ic_->waitForShutdown();
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
}

MWRegistryProxy IceMiddleware::create_registry_proxy(string const& identity, string const& endpoint)
{
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
}

MWScopeProxy IceMiddleware::create_scope_proxy(string const& identity, string const& endpoint)
{
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
}

MWQueryCtrlProxy IceMiddleware::add_query_ctrl_object(QueryCtrlObject::SPtr const& ctrl)
{
    assert(ctrl);

    MWQueryCtrlProxy proxy;
    try
    {
        QueryCtrlIPtr ci = new QueryCtrlI(ctrl);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_ + ctrl_suffix);
        proxy.reset(new IceQueryCtrl(this, middleware::QueryCtrlPrx::uncheckedCast(safe_add(a, ci))));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

MWQueryProxy IceMiddleware::add_query_object(QueryObject::SPtr const& query)
{
    assert(query);

    MWQueryProxy proxy;
    try
    {
        QueryIPtr qi = new QueryI(this, query);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_);
        proxy.reset(new IceQuery(this, middleware::QueryPrx::uncheckedCast(safe_add(a, qi))));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}


MWRegistryProxy IceMiddleware::add_registry_object(string const& identity, RegistryObject::SPtr const& registry)
{
    assert(!identity.empty());
    assert(registry);

    MWRegistryProxy proxy;
    try
    {
        RegistryIPtr ri = new RegistryI(registry);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_);
        proxy.reset(new IceRegistry(this, middleware::RegistryPrx::uncheckedCast(safe_add(a, ri, identity))));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

MWReplyProxy IceMiddleware::add_reply_object(ReplyObject::SPtr const& reply)
{
    assert(reply);

    MWReplyProxy proxy;
    try
    {
        ReplyIPtr si = new ReplyI(reply);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_ + reply_suffix);
        proxy.reset(new IceReply(this, middleware::ReplyPrx::uncheckedCast(safe_add(a, si))));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

MWScopeProxy IceMiddleware::add_scope_object(string const& identity, ScopeObject::SPtr const& scope)
{
    assert(!identity.empty());
    assert(scope);

    MWScopeProxy proxy;
    try
    {
        ScopeIPtr si = new ScopeI(this, scope);
        Ice::ObjectAdapterPtr a = find_adapter(server_name_ + ctrl_suffix);
        proxy.reset(new IceScope(this, middleware::ScopePrx::uncheckedCast(safe_add(a, si, identity))));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

#if 0
void IceMiddleware::remove_object(string const& identity)
{
    try
    {
        Ice::Identity id;
        id.name = identity;

        lock_guard<mutex> lock(mutex_);
        if (!adapter_)
        {
            throw LogicException("Cannot remove object from stopped middleware");
        }
        adapter_->remove(id);
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
}
#endif

Ice::ObjectAdapterPtr IceMiddleware::find_adapter(string const& name)
{
    lock_guard<mutex> lock(mutex_);

    auto it = am_.find(name);
    if (it != am_.end())
    {
        return it->second;
    }

    Ice::ObjectAdapterPtr a = ic_->createObjectAdapter(name);
    a->activate();
    am_[name] = a;
    return a;
}

Ice::ObjectPrx IceMiddleware::safe_add(Ice::ObjectAdapterPtr& adapter,
                                       Ice::ObjectPtr const& obj,
                                       string const& identity)
{
    Ice::Identity id;
    id.name = identity.empty() ? IceUtil::generateUUID() : identity;

    lock_guard<mutex> lock(mutex_);
    if (!adapter)
    {
        throw LogicException("Cannot add object to stopped middleware");
    }
    return adapter->add(obj, id);
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
