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
#include <unity/api/scopes/internal/RegistryImpl.h>
#include <unity/api/scopes/internal/RuntimeConfig.h>
#include <unity/api/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <cassert>

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
    destroyed_(false),
    scope_name_(scope_name)
{
    if (scope_name.empty())
    {
        throw InvalidArgumentException("Cannot instantiate a run time with an empty scope name");
    }

    try
    {
        // Create the middleware factory and get the registry identity and config filename.
        RuntimeConfig config(configfile);
        string default_middleware = config.default_middleware();
        string middleware_configfile = config.default_middleware_configfile();
        string factory_configfile = config.factory_configfile();
        middleware_factory_.reset(new MiddlewareFactory(factory_configfile, this));
        registry_configfile_ = config.registry_configfile();
        registry_identity_ = config.registry_identity();
        assert(!registry_identity_.empty());

        middleware_ = middleware_factory_->create(scope_name, default_middleware, middleware_configfile);
        middleware_->start();

        // Create the registry proxy.
        RegistryConfig reg_config(registry_identity_, registry_configfile_);
        string reg_mw_configfile = reg_config.mw_configfile();
        string reg_endpoint = reg_config.endpoint();

        auto registry_mw_proxy = middleware_->create_registry_proxy(registry_identity_, reg_endpoint);
        registry_ = RegistryImpl::create(registry_mw_proxy, this);
    }
    catch (unity::Exception const& e)
    {
        throw ConfigException("Cannot instantiate run time for " + scope_name + ", config file: " + configfile);
    }
}

RuntimeImpl::~RuntimeImpl() noexcept
{
    try
    {
        destroy();
    }
    catch (unity::Exception const& e) // LCOV_EXCL_LINE
    {
        // TODO: log error
    }
    catch (...) // LCOV_EXCL_LINE
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
        reply_reaper_ = nullptr;
    }
}

string RuntimeImpl::scope_name() const
{
    return scope_name_;
}

MiddlewareFactory const* RuntimeImpl::factory() const
{
    if (destroyed_.load())
    {
        throw LogicException("factory(): Cannot obtain factory for already destroyed run time");
    }
    return middleware_factory_.get();
}

RegistryProxy RuntimeImpl::registry() const
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

string RuntimeImpl::registry_identity() const
{
    return registry_identity_;
}

Reaper::SPtr RuntimeImpl::reply_reaper() const
{
    // We lazily create a reaper the first time we are asked for it, which happens when the first query is created.
    lock_guard<mutex> lock(mutex_);
    if (!reply_reaper_)
    {
        reply_reaper_ = Reaper::create(1, 5); // TODO: configurable timeouts
    }
    return reply_reaper_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
