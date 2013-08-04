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

#include <unity/api/scopes/internal/ice_middleware/IceRegistryProxy.h>
#include <unity/api/scopes/internal/ice_middleware/IceScopeProxy.h>
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

IceMiddleware::IceMiddleware(string const& server_name, string const& configfile)
{
    // For an empty server name, we use a UUID is the server name (which becomes the adapter
    // name). This allows us to have clients (such as the Dash) that are not scopes themselves
    // and do not need a well-known endpoint to receive incoming queries.
    if (server_name.empty())
    {
        server_name_ = IceUtil::generateUUID();
    }
    else
    {
        server_name_ = server_name;
    }

    try
    {
        Ice::PropertiesPtr props = Ice::createProperties();
        props->load(configfile);
        if (server_name.empty())
        {
            // TODO: get directory from config
            props->setProperty(server_name_ + ".Endpoints",  "uds -f /tmp/scope-client-" + server_name_);
        }
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
    try
    {
        lock_guard<mutex> lock(mutex_);
        adapter_ = ic_->createObjectAdapter(server_name_);
        adapter_->activate();
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
}

void IceMiddleware::stop()
{
    try
    {
        // TODO: need to cancel any outstanding stuff here?
        // TODO: need to disable proxy?
        Ice::ObjectAdapterPtr a;
        {
            lock_guard<mutex> lock(mutex_);
            a = adapter_;
            adapter_ = nullptr;
        }
        a->deactivate();
        a->waitForDeactivate();
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

MWRegistryProxy::SPtr IceMiddleware::create_registry_proxy(string const& identity, string const& endpoint)
{
    MWRegistryProxy::SPtr proxy;
    try
    {
        string proxyString = identity + ":" + endpoint;
        Ice::ObjectPrx o = ic_->stringToProxy(proxyString);
        if (!o)
        {
            // TODO: throw config exception
            assert(false);
        }
        proxy.reset(new IceRegistryProxy(this, o));
    }
    catch (Ice::Exception const& e)
    {
        rethrow_ice_ex(e);
    }
    return proxy;
}

MWScopeProxy::SPtr IceMiddleware::create_scope_proxy(string const& identity, string const& endpoint)
{
    MWScopeProxy::SPtr proxy;
    try
    {
        string proxy_string = identity + ":" + endpoint;
        Ice::ObjectPrx o = ic_->stringToProxy(proxy_string);
        if (!o)
        {
            throw ConfigException("Ice: could not create proxy from string \"" + proxy_string + "\"");
        }
        proxy.reset(new IceScopeProxy(this, o));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

MWRegistryProxy::SPtr IceMiddleware::add_registry_object(string const& identity, RegistryObject::SPtr const& registry)
{
    assert(!identity.empty());
    assert(registry);

    MWRegistryProxy::SPtr proxy;
    try
    {
        RegistryIPtr ri = new RegistryI(registry);
        proxy.reset(new IceRegistryProxy(this, safe_add(ri, identity)));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

MWScopeProxy::SPtr IceMiddleware::add_scope_object(string const& identity, ScopeObject::SPtr const& scope)
{
    assert(!identity.empty());
    assert(scope);

    MWScopeProxy::SPtr proxy;
    try
    {
        ScopeIPtr si = new ScopeI(this, scope);
        proxy.reset(new IceScopeProxy(this, safe_add(si, identity)));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

MWReplyProxy::SPtr IceMiddleware::add_reply_object(ReplyObject::SPtr const& reply)
{
    assert(reply);

    MWReplyProxy::SPtr proxy;
    try
    {
        ReplyIPtr si = new ReplyI(reply);
        proxy.reset(new IceReplyProxy(this, safe_add(si)));
    }
    catch (Ice::Exception const& e)
    {
        // TODO: log exception
        rethrow_ice_ex(e);
    }
    return proxy;
}

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

Ice::ObjectPrx IceMiddleware::safe_add(Ice::ObjectPtr const& obj, string const& identity)
{
    Ice::Identity id;
    id.name = identity.empty() ? IceUtil::generateUUID() : identity;

    lock_guard<mutex> lock(mutex_);
    if (!adapter_)
    {
        throw LogicException("Cannot add object to stopped middleware");
    }
    return adapter_->add(obj, id);
}

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
