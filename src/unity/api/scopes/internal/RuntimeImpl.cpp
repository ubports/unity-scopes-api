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

#include <unity/api/scopes/internal/RuntimeImpl.h>

#include <unity/api/scopes/internal/RegistryConfig.h>
#include <unity/api/scopes/internal/RegistryProxyImpl.h>
#include <unity/api/scopes/internal/RuntimeConfig.h>
#include <unity/api/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;
using namespace unity::api::scopes;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

RuntimeImpl::RuntimeImpl(string const& scope_name, string const& configfile) :
    destroyed_(false)
{
    // Create the middleware factory and get the registry identity and config filename.
    string registry_identity;
    try
    {
        RuntimeConfig config(configfile);
        string default_middleware = config.default_middleware();
        string factory_configfile = config.factory_configfile();
        middleware_factory_.reset(new MiddlewareFactory(factory_configfile));
        registry_configfile_ = config.registry_configfile();
        registry_identity = config.registry_identity();
    }
    catch (unity::Exception const& e)
    {
        throw ConfigException("cannot create MiddlewareFactory: config file: " + configfile);
    }

    // Create the registry proxy.
    try
    {
        RegistryConfig config(registry_identity, registry_configfile_);
        string identity = config.identity();
        string kind = config.mw_kind();
        string configfile = config.mw_configfile();
        string endpoint = config.endpoint();

        middleware_ = middleware_factory_->create(scope_name, kind, configfile);
        middleware_->start();
        auto registry_proxy = middleware_->create_registry_proxy(identity, endpoint);
        registry_ = RegistryProxyImpl::create(registry_proxy);
    }
    catch (unity::Exception const& e)
    {
        throw ConfigException("cannot create registry proxy: config file: " + registry_configfile_);
    }
}

RuntimeImpl::~RuntimeImpl() noexcept
{
    try
    {
        destroy();
    }
    catch (unity::Exception const& e)
    {
        // TODO: log error
    }
    catch (...)
    {
        // TODO: log error
    }
}

RuntimeImpl::UPtr RuntimeImpl::create(string const& scope_name, string const& configfile)
{
    return UPtr(new RuntimeImpl(scope_name, configfile));
}

void RuntimeImpl::destroy()
{
    if (!destroyed_.exchange(true))
    {
        registry_ = nullptr;
        middleware_->stop();
        middleware_ = nullptr;
        middleware_factory_.reset(nullptr);
    }
}

MiddlewareFactory const* RuntimeImpl::factory() const
{
    if (destroyed_.load())
    {
        throw LogicException("factory(): Cannot obtain factory for already destroyed run time");
    }
    return middleware_factory_.get();
}

RegistryProxy::SPtr RuntimeImpl::registry() const
{
    if (destroyed_.load())
    {
        throw LogicException("registry(): Cannot obtain registry for already destroyed run time");
    }
    return registry_;
}

string RuntimeImpl::registry_configfile() const
{
    return registry_configfile_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
