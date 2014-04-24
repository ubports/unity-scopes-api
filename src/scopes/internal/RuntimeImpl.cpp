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
#include <unity/scopes/internal/ScopeBaseImpl.h>

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/internal/MWStateReceiver.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RegistryImpl.h>
#include <unity/scopes/internal/RuntimeConfig.h>
#include <unity/scopes/internal/ScopeObject.h>
#include <unity/scopes/internal/UniqueID.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <signal.h>
#include <libgen.h>

#include <cassert>
#include <cstring>
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

RuntimeImpl::RuntimeImpl(string const& scope_id, string const& configfile)
{
    lock_guard<mutex> lock(mutex_);

    destroyed_ = false;

    scope_id_ = scope_id;
    if (scope_id_.empty())
    {
        UniqueID id;
        scope_id_ = "c-" + id.gen();
    }

    string config_file(configfile.empty() ? DFLT_RUNTIME_INI : configfile);
    configfile_ = config_file;

    try
    {
        // Create the middleware factory and get the registry identity and config filename.
        RuntimeConfig config(config_file);
        string default_middleware = config.default_middleware();
        string middleware_configfile = config.default_middleware_configfile();
        middleware_factory_.reset(new MiddlewareFactory(this));
        registry_configfile_ = config.registry_configfile();
        registry_identity_ = config.registry_identity();

        middleware_ = middleware_factory_->create(scope_id_, default_middleware, middleware_configfile);
        middleware_->start();

        async_pool_ = make_shared<ThreadPool>(1); // TODO: configurable pool size
        future_queue_ = make_shared<ThreadSafeQueue<future<void>>>();
        waiter_thread_ = std::thread([this]{ waiter_thread(future_queue_); });

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
            registry_ = make_shared<RegistryImpl>(registry_mw_proxy, this);
        }
    }
    catch (unity::Exception const& e)
    {
        throw ConfigException("Cannot instantiate run time for " + scope_id + ", config file: " + config_file);
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

RuntimeImpl::UPtr RuntimeImpl::create(string const& scope_id, string const& configfile)
{
    return UPtr(new RuntimeImpl(scope_id, configfile));
}

void RuntimeImpl::destroy()
{
    lock_guard<mutex> lock(mutex_);

    if (destroyed_)
    {
        return;
    }
    destroyed_ = true;

    // Stop the reaper. Any ReplyObject instances that may still
    // be hanging around will be cleaned up when the server side
    // is shut down.
    if (reply_reaper_)
    {
        reply_reaper_->destroy();
        reply_reaper_ = nullptr;
    }

    // No more outgoing invocations.
    async_pool_ = nullptr;

    // Wait for any twoway operations that were invoked asynchronously to complete.
    if (future_queue_)
    {
        future_queue_->wait_until_empty();
        future_queue_->destroy();
        future_queue_->wait_for_destroy();
    }

    if (waiter_thread_.joinable())
    {
        waiter_thread_.join();
    }

    // Shut down server-side.
    middleware_->stop();
    middleware_->wait_for_shutdown();
    middleware_ = nullptr;
    middleware_factory_.reset(nullptr);

    registry_ = nullptr;
}

string RuntimeImpl::scope_id() const
{
    lock_guard<mutex> lock(mutex_);
    return scope_id_;
}

string RuntimeImpl::configfile() const
{
    return configfile_;
}

MiddlewareFactory const* RuntimeImpl::factory() const
{
    lock_guard<mutex> lock(mutex_);

    if (destroyed_)
    {
        throw LogicException("factory(): Cannot obtain factory for already destroyed run time");
    }
    return middleware_factory_.get();
}

RegistryProxy RuntimeImpl::registry() const
{
    lock_guard<mutex> lock(mutex_);

    if (destroyed_)
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
    lock_guard<mutex> lock(mutex_);
    return registry_configfile_;
}

string RuntimeImpl::registry_identity() const
{
    lock_guard<mutex> lock(mutex_);
    return registry_identity_;
}

string RuntimeImpl::registry_endpointdir() const
{
    lock_guard<mutex> lock(mutex_);
    return registry_endpointdir_;
}

string RuntimeImpl::registry_endpoint() const
{
    lock_guard<mutex> lock(mutex_);
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

void RuntimeImpl::waiter_thread(ThreadSafeQueue<std::future<void>>::SPtr const& queue) const noexcept
{
    for (;;)
    {
        try
        {
            // Wait on the future from an async invocation.
            // wait_and_pop() throws runtime_error when queue is destroyed
            queue->wait_and_pop().get();
            cerr << "waiter: got item" << endl;
        }
        catch (std::runtime_error const&)
        {
            break;
        }
    }
}

ThreadPool::SPtr RuntimeImpl::async_pool() const
{
    lock_guard<mutex> lock(mutex_);
    return async_pool_;
}

ThreadSafeQueue<future<void>>::SPtr RuntimeImpl::future_queue() const
{
    lock_guard<mutex> lock(mutex_);
    return future_queue_;
}

void RuntimeImpl::run_scope(ScopeBase *const scope_base, std::string const& scope_ini_file)
{
    // Retrieve the registry middleware and create a proxy to its state receiver
    RegistryConfig reg_conf(registry_identity_, registry_configfile_);
    auto reg_runtime = create(registry_identity_, configfile_);
    auto reg_mw = reg_runtime->factory()->find(registry_identity_, reg_conf.mw_kind());
    auto reg_state_receiver = reg_mw->create_state_receiver_proxy("StateReceiver");

    auto mw = factory()->create(scope_id_, reg_conf.mw_kind(), reg_conf.mw_configfile());

    {
        // dirname modifies its argument, so we need a copy of scope lib path
        std::vector<char> scope_ini(scope_ini_file.c_str(), scope_ini_file.c_str() + scope_ini_file.size() + 1);
        const std::string scope_dir(dirname(&scope_ini[0]));
        scope_base->p->set_scope_directory(scope_dir);
    }

    scope_base->start(scope_id_, registry());
    // Ensure the scope gets stopped.
    unique_ptr<ScopeBase, void(*)(ScopeBase*)> cleanup_scope(scope_base, [](ScopeBase *scope_base) { scope_base->stop(); });

    // Give a thread to the scope to do with as it likes. If the scope
    // doesn't want to use it and immediately returns from run(),
    // that's fine.
    auto run_future = std::async(launch::async, [scope_base] { scope_base->run(); });

    // Create a servant for the scope and register the servant.
    auto scope = unique_ptr<internal::ScopeObject>(new internal::ScopeObject(this, scope_base));
    auto proxy = mw->add_scope_object(scope_id_, move(scope));

    // Inform the registry that this scope is now ready to process requests
    reg_state_receiver->push_state(scope_id_, StateReceiverObject::State::ScopeReady);

    mw->wait_for_shutdown();

    // Inform the registry that this scope is shutting down
    reg_state_receiver->push_state(scope_id_, StateReceiverObject::State::ScopeStopping);

    run_future.get();
}

ObjectProxy RuntimeImpl::string_to_proxy(string const& s) const
{
    auto mw = middleware_factory_->find(s);
    assert(mw);
    return mw->string_to_proxy(s);
}

string RuntimeImpl::proxy_to_string(ObjectProxy const& proxy) const
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
