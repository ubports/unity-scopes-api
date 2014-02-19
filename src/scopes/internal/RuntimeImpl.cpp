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

#include <unity/scopes/internal/RuntimeImpl.h>

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RegistryImpl.h>
#include <unity/scopes/internal/RuntimeConfig.h>
#include <unity/scopes/internal/ScopeObject.h>
#include <unity/scopes/internal/UniqueID.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <future>
#include <iostream> // TODO: remove this once logging is added


using namespace std;
using namespace unity::scopes;

namespace unity
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
        UniqueID id;
        scope_name_ = "c-" + id.gen();
    }

    string config_file(configfile.empty() ? DFLT_RUNTIME_INI : configfile);

    try
    {
        // Create the middleware factory and get the registry identity and config filename.
        RuntimeConfig config(config_file);
        string default_middleware = config.default_middleware();
        string middleware_configfile = config.default_middleware_configfile();
        middleware_factory_.reset(new MiddlewareFactory(this));
        registry_configfile_ = config.registry_configfile();
        registry_identity_ = config.registry_identity();

        middleware_ = middleware_factory_->create(scope_name_, default_middleware, middleware_configfile);
        middleware_->start();

        if (registry_configfile_.empty() || registry_identity_.empty())
        {
            cerr << "Warning: no registry configured" << endl;
            registry_identity_ = "";
        }
        else
        {
            // Create the registry proxy.
            RegistryConfig reg_config(registry_identity_, registry_configfile_);
            string reg_mw_configfile = reg_config.mw_configfile();
            registry_endpoint_ = reg_config.endpoint();
            registry_endpointdir_ = reg_config.endpointdir();
            auto registry_mw_proxy = middleware_->create_registry_proxy(registry_identity_, registry_endpoint_);
            registry_ = RegistryImpl::create(registry_mw_proxy, this);
cerr << "runtime created" << endl;
        }
    }
    catch (unity::Exception const& e)
    {
        throw ConfigException("Cannot instantiate run time for " + scope_name + ", config file: " + config_file);
    }
}

RuntimeImpl::~RuntimeImpl()
{
    try
    {
        destroy();
    }
    catch (std::exception const& e) // LCOV_EXCL_LINE
    {
        cerr << "~RuntimeImpl(): " << e.what() << endl;
        // TODO: log error
    }
    catch (...) // LCOV_EXCL_LINE
    {
        cerr << "~RuntimeImpl(): unknown exception" << endl;
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
        // TODO: not good enough. Need to wait for the middleware to stop and for the reaper
        // to terminate. Otherwise, it's possible that we exit while threads are still running
        // with undefined behavior.
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
    if (!registry_)
    {
        throw ConfigException("registry(): no registry configured");
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

string RuntimeImpl::registry_endpointdir() const
{
    return registry_endpointdir_;
}

string RuntimeImpl::registry_endpoint() const
{
    return registry_endpoint_;
}

Reaper::SPtr RuntimeImpl::reply_reaper() const
{
    // We lazily create a reaper the first time we are asked for it, which happens when the first query is created.
    lock_guard<mutex> lock(mutex_);
    if (!reply_reaper_)
    {
        reply_reaper_ = Reaper::create(10, 45); // TODO: configurable timeouts
    }
    return reply_reaper_;
}

void RuntimeImpl::run_scope(ScopeBase *const scope_base)
{
    auto mw = factory()->create(scope_name_, "Zmq", "Zmq.ini");

    scope_base->start(scope_name_, registry());
    // Ensure the scope gets stopped.
    unique_ptr<ScopeBase, void(*)(ScopeBase*)> cleanup_scope(scope_base, [](ScopeBase *scope_base) { scope_base->stop(); });

    // Give a thread to the scope to do with as it likes. If the scope
    // doesn't want to use it and immediately returns from run(),
    // that's fine.
    auto run_future = std::async(launch::async, [scope_base] { scope_base->run(); });

    // Create a servant for the scope and register the servant.
    auto scope = unique_ptr<internal::ScopeObject>(new internal::ScopeObject(this, scope_base));
    auto proxy = mw->add_scope_object(scope_name_, move(scope));

    mw->wait_for_shutdown();
    run_future.get();
}

Proxy RuntimeImpl::string_to_proxy(string const& s) const
{
    auto mw = middleware_factory_->find(s);
    assert(mw);
    return mw->string_to_proxy(s);
}

string RuntimeImpl::proxy_to_string(Proxy const& proxy) const
{
    if (proxy == nullptr)
    {
        return "nullproxy:";
    }
    return proxy->to_string();
}

} // namespace internal

} // namespace scopes

} // namespace unity
