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

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/RegistryImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/zmq_middleware/ConnectionPool.h>
#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/internal/zmq_middleware/QueryI.h>
#include <unity/scopes/internal/zmq_middleware/QueryCtrlI.h>
#include <unity/scopes/internal/zmq_middleware/RegistryI.h>
#include <unity/scopes/internal/zmq_middleware/ReplyI.h>
#include <unity/scopes/internal/zmq_middleware/ScopeI.h>
#include <unity/scopes/internal/zmq_middleware/StateReceiverI.h>
#include <unity/scopes/internal/zmq_middleware/ZmqConfig.h>
#include <unity/scopes/internal/zmq_middleware/ZmqPublisher.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQuery.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqRegistry.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/scopes/internal/zmq_middleware/ZmqStateReceiver.h>
#include <unity/scopes/internal/zmq_middleware/ZmqSubscriber.h>
#include <unity/scopes/internal/zmq_middleware/RethrowException.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <iostream>  // TODO: remove this once logging is added
#include <sys/stat.h>

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

char const* query_suffix = "-q";     // Appended to server_name_ to create query adapter name
char const* ctrl_suffix = "-c";      // Appended to server_name_ to create control adapter name
char const* reply_suffix = "-r";     // Appended to server_name_ to create reply adapter name
char const* state_suffix = "-s";     // Appended to server_name_ to create state adapter name
char const* registry_suffix = "-R";  // Appended to server_name_ to create registry (or SS registry) adapter name
char const* publisher_suffix = "-p"; // Appended to publisher_id to create a publisher endpoint

char const* query_category = "Query";       // query adapter category name
char const* ctrl_category = "QueryCtrl";    // control adapter category name
char const* reply_category = "Reply";       // reply adapter category name
char const* state_category = "State";       // state adapter category name
char const* scope_category = "Scope";       // scope adapter category name
char const* registry_category = "Registry"; // registry adapter category name

// Create a directory with the given mode if it doesn't exist yet.

void create_dir(string const& dir, mode_t mode)
{
    if (mkdir(dir.c_str(), mode) == -1 && errno != EEXIST)
    {
        throw FileException("cannot create endpoint directory " + dir, errno);
    }
}

} // namespace

ZmqMiddleware::ZmqMiddleware(string const& server_name, RuntimeImpl* runtime, string const& configfile) :
    MiddlewareBase(runtime),
    server_name_(server_name),
    state_(Created),
    shutdown_flag(false)
{
    assert(!server_name.empty());

    try
    {
        ZmqConfig config(configfile);

        twoway_timeout_ = config.twoway_timeout();
        locate_timeout_ = config.locate_timeout();
        registry_timeout_ = config.registry_timeout();
        public_endpoint_dir_ = config.endpoint_dir();
        private_endpoint_dir_ = public_endpoint_dir_ + "/priv";
        registry_endpoint_dir_ = public_endpoint_dir_;
        ss_registry_endpoint_dir_ = public_endpoint_dir_;
        registry_identity_ = DFLT_REGISTRY_ID;
        ss_registry_identity_ = DFLT_SS_REGISTRY_ID;

        if (runtime) // runtime can be nullptr for some of the tests. It is never null otherwise.
        {
            // The registry and smartscopesproxy can override the normal endpoint dir. That's needed
            // for testing, so the shell can run a test registry somewhere other than the normal
            // registry, but still use the normal smartscopesproxy.
            if (!config.registry_endpoint_dir().empty())
            {
                registry_endpoint_dir_ = config.registry_endpoint_dir();
            }
            if (!config.ss_registry_endpoint_dir().empty())
            {
                ss_registry_endpoint_dir_ = config.ss_registry_endpoint_dir();
            }
            registry_identity_ = runtime->registry_identity();
            ss_registry_identity_ = runtime->ss_registry_identity();
        }

        // Create the endpoint dirs if they don't exist.
        // We set the sticky bit because, without this, things in
        // $XDG_RUNTIME_DIR may be deleted if not accessed for more than six hours.
        create_dir(public_endpoint_dir_, 0755 | S_ISVTX);
        create_dir(private_endpoint_dir_, 0700 | S_ISVTX);
        create_dir(registry_endpoint_dir_, 0755 | S_ISVTX);
        create_dir(ss_registry_endpoint_dir_, 0755 | S_ISVTX);
    }
    catch (...)
    {
        throw MiddlewareException("cannot initialize zmq middleware for scope " + server_name);
    }
}

ZmqMiddleware::~ZmqMiddleware()
{
    try
    {
        stop();
        wait_for_shutdown();

        // TODO:
        // We terminate explicitly here instead of relying
        // on the context_ destructor so we can measure how long it
        // takes. There is an intermittent problem with
        // zmq taking several seconds to terminate the context.
        // Until we figure out what's going on here, we measure
        // how long it takes and print a warning if it takes
        // longer than 100 ms.
        auto start_time = chrono::system_clock::now();
        context_.terminate();
        auto end_time = chrono::system_clock::now();
        auto millisecs = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
        if (millisecs > 100)
        {
            ostringstream s;
            s << "warning: ~ZmqMiddleware(): context_.terminate() took " << millisecs
              << " ms to complete for " << server_name_ << endl;
            cerr << s.str();
        }
    }
    catch (std::exception const& e)
    {
        cerr << "~ZmqMiddleware(): " << e.what() << endl;
        // TODO: log exception
    }
    catch (...)
    {
        cerr << "~ZmqMiddleware(): unknown exception" << endl;
        // TODO: log exception
    }
}

void ZmqMiddleware::start()
{
    unique_lock<mutex> lock(state_mutex_);

    switch (state_)
    {
        case Started:
        {
            return; // Already started, no-op
        }
        case Stopping:
        case Stopped:
        {
            throw MiddlewareException("Cannot re-start stopped middleware");
        }
        case Created:
        {
            {
                lock_guard<mutex> lock(data_mutex_);
                try
                {
                    oneway_invoker_.reset(new ThreadPool(1));  // Oneway pool must have a single thread
                    // N.B. We absolutely MUST have AT LEAST 5 two-way invoke threads:
                    // * 3 threads are required to execute a standard scope invocation as both
                    //   rebinding and debug_mode requests could be invoked within a single two-way
                    //   invocation.
                    // * We then need an extra thread available per layer of hierarchy under an
                    //   aggregating scope as an aggregator may invoke a nested scope while running in
                    //   a thread of its own.
                    // * 5 threads therefore, at least allows for an aggregating scope to invoke nested
                    //   aggregators.
                    // (NOTE: To be safe, we should keep some headroom above this 5 thread minimum)
                    twoway_invokers_.reset(new ThreadPool(8));  // TODO: get pool size from config
                }
                catch (std::exception const& e)
                {
                    throw MiddlewareException(string("Cannot create outgoing invocation pools: ") + e.what());
                }
                catch (...)
                {
                    throw MiddlewareException("Cannot create outgoing invocation pools: unknown exception");
                }
            }
            shutdown_flag = false;
            state_ = Started;
            state_changed_.notify_all();
            break;
        }
        default:
        {
            assert(false);  // LCOV_EXCL_LINE
        }
    }
}

void ZmqMiddleware::stop()
{
    unique_lock<mutex> lock(state_mutex_);
    switch (state_)
    {
        case Stopped:
        case Stopping:
        {
            break;  // Already stopped, about to stop, or never started: no-op
        }
        case Created:
        case Started:
        {
            lock_guard<mutex> lock(data_mutex_);

            // No more outgoing invocations
            assert((oneway_invoker_ && twoway_invokers_) || (!oneway_invoker_ && !twoway_invokers_));
            if (oneway_invoker_)
            {
                twoway_invokers_->destroy();            // Destroy immediately, because invocations can take time.
                oneway_invoker_->destroy_once_empty();  // Wait for queued oneways to go out first.
            }

            // Initiate shutdown of all adapters
            for (auto& pair : am_)
            {
                pair.second->shutdown();
            }

            state_ = Stopping;
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
    {
        unique_lock<mutex> state_lock(state_mutex_);
        state_changed_.wait(state_lock, [this] {
            return state_ == Stopping || state_ == Stopped || state_ == Created;
        });
        if (state_ == Stopped)
        {
            return; // Return immediately if stopped already, or never started in the first place.
        }

        // Several threads may see state_ == Stopping here. Exactly one of them
        // waits for the object adapters to shut down; the others wait until
        // shut down is complete.
        if (shutdown_flag.exchange(true))
        {
            // Another thread has been through here already, wait for it
            // to finish shutting down the adapters.
            state_changed_.wait(state_lock, [this] { return state_ == Stopped; });
            return;
        }
    }

    // Exactly one thread gets to this point.
    AdapterMap adapter_map;
    {
        lock_guard<mutex> data_lock(data_mutex_);
        adapter_map = move(am_);
    }
    for (auto&& pair : adapter_map)
    {
        pair.second->wait_for_shutdown();
    }

    unique_lock<mutex> state_lock(state_mutex_);
    state_ = Stopped;
    state_changed_.notify_all();
}

namespace
{

void bad_proxy_string(string const& msg)
{
    throw MiddlewareException("string_to_proxy(): " + msg);
}

}

// Poor man's URI parser (no boost)

ObjectProxy ZmqMiddleware::string_to_proxy(string const& s)
{
    if (s == "nullproxy:")
    {
        return nullptr;
    }
    if (s.empty())
    {
        bad_proxy_string("proxy string cannot be empty");
    }
    static const string scheme = "ipc://";
    if (s.substr(0, scheme.size()) != scheme)
    {
        bad_proxy_string("invalid proxy scheme prefix: \"" + s + "\" (expected \"" + scheme + "\")");
    }
    auto fragment_pos = s.find_first_of('#');
    if (fragment_pos == string::npos)
    {
        bad_proxy_string("invalid proxy: missing # separator: " + s);
    }

    string endpoint(s.begin(), s.begin() + fragment_pos);  // Everything up to the '#'
    if (endpoint.size() == scheme.size())
    {
        bad_proxy_string("invalid proxy: empty endpoint path: " + s);
    }

    string fields(s.substr(fragment_pos + 1));      // Everything following the '#'

    auto excl_pos = fields.find_first_of('!');
    auto end_it = excl_pos == string::npos ? fields.end() : fields.begin() + excl_pos;

    string identity(fields.begin(), end_it);
    if (identity.empty())
    {
        bad_proxy_string("invalid proxy: empty identity: " + s);
    }

    fields = fields.substr(identity.size());

    // Remaining fields are optional. Field assignments are separated by '!'
    // Collect field assignments in fvals.
    vector<string> fvals;
    while (!fields.empty())
    {
        if (fields[0] == '!')
        {
            fields = fields.substr(1);
        }
        excl_pos = fields.find_first_of('!');
        end_it = excl_pos == string::npos ? fields.end() : fields.begin() + excl_pos;
        string val(fields.begin(), end_it);
        if (val.empty())
        {
            bad_proxy_string("invalid proxy: invalid empty field specification: " + s);
        }
        fvals.push_back(val);
        fields = fields.substr(val.size());
    }

    // fvals now contains field assignments. Insert each assignment into a map, so we
    // check for duplicate fields.
    map<char, string> fmap;
    const string valid_fields = "cmt";
    for (auto const& v : fvals)
    {
        if (v.size() < 2 || v[1] != '=')
        {
            bad_proxy_string("invalid proxy: bad field specification (\"" + v + "\"): " + s);
        }
        if (valid_fields.find(v[0]) == string::npos)
        {
            bad_proxy_string("invalid proxy: invalid field identifier (\"" + v + "\"): " + s);
        }
        if (fmap.find(v[0]) != fmap.end())
        {
            bad_proxy_string("invalid proxy: duplicate field specification (\"" + string(1, v[0]) + "=\"): " + s);
        }
        fmap[v[0]] = string(v.begin() + 2, v.end());
    }

    // Now run over the map and check each value
    string category = scope_category;
    RequestMode mode = RequestMode::Twoway;
    int64_t timeout = -1;
    for (auto const& pair : fmap)
    {
        switch (pair.first)
        {
            case 'c':
            {
                category = pair.second;  // Empty category is OK
                break;
            }
            case 'm':
            {
                if (pair.second.empty() || (pair.second != "o" && pair.second != "t"))
                {
                    bad_proxy_string("invalid proxy: bad mode (\"m=" + pair.second + "\"): " + s);
                }
                mode = pair.second == "o" ? RequestMode::Oneway : RequestMode::Twoway;
                break;
            }
            case 't':
            {
                if (pair.second.empty())
                {
                    bad_proxy_string("invalid proxy: bad timeout value (\"t=" + pair.second + "\"): " + s);
                }
                size_t pos;
                try
                {
                    timeout = std::stol(pair.second, &pos);
                }
                catch (std::exception const&)
                {
                    pos = 0;
                }
                if (pair.second[pos] != '\0')  // Did not consume all of the assignment
                {
                    bad_proxy_string("invalid proxy: bad timeout value (\"t=" + pair.second + "\"): " + s);
                }
                if (timeout < -1)
                {
                    bad_proxy_string("invalid proxy: bad timeout value (\"t=" + pair.second + "\"): " + s);
                }
                break;
            }
            default:
            {
                assert(false);  // LCOV_EXCL_LINE
            }
        }
    }

    return make_typed_proxy(endpoint, identity, category, mode, timeout);
}

string ZmqMiddleware::proxy_to_string(MWProxy const& proxy)
{
    if (!proxy)
    {
        return "nullproxy:";
    }
    return proxy->to_string();
}

MWRegistryProxy ZmqMiddleware::registry_proxy()
{
    lock_guard<mutex> lock(data_mutex_);

    if (!registry_proxy_)
    {
        string r_id = registry_identity_;  // May be empty, if no registry is configured.
        if (!r_id.empty())
        {
            string r_endp = "ipc://" + registry_endpoint_dir_ + "/" + r_id + registry_suffix;
            registry_proxy_.reset(new ZmqRegistry(this, r_endp, r_id, registry_category, twoway_timeout_));
        }
    }
    return registry_proxy_;
}

MWRegistryProxy ZmqMiddleware::ss_registry_proxy()
{
    lock_guard<mutex> lock(data_mutex_);

    if (!ss_registry_proxy_)
    {
        string ssr_id = ss_registry_identity_;  // May be empty, if no ss registry is configured.
        if (!ssr_id.empty())
        {
            string ssr_endp = "ipc://" + ss_registry_endpoint_dir_ + "/" + ssr_id + registry_suffix;
            ss_registry_proxy_.reset(new ZmqRegistry(this, ssr_endp, ssr_id, registry_category, twoway_timeout_));
        }
    }
    return ss_registry_proxy_;
}

MWScopeProxy ZmqMiddleware::create_scope_proxy(string const& identity)
{
    string endpoint = "ipc://" + private_endpoint_dir_ + "/" + identity;
    return make_shared<ZmqScope>(this, endpoint, identity, scope_category, twoway_timeout_);
}

MWScopeProxy ZmqMiddleware::create_scope_proxy(string const& identity, string const& endpoint)
{
    return make_shared<ZmqScope>(this, endpoint, identity, scope_category, twoway_timeout_);
}

MWQueryProxy ZmqMiddleware::create_query_proxy(string const& identity, string const& endpoint)
{
    return make_shared<ZmqQuery>(this, endpoint, identity, query_category);
}

MWQueryCtrlProxy ZmqMiddleware::create_query_ctrl_proxy(string const& identity, string const& endpoint)
{
    return make_shared<ZmqQueryCtrl>(this, endpoint, identity, ctrl_category);
}

MWStateReceiverProxy ZmqMiddleware::create_state_receiver_proxy(std::string const& identity)
{
    // Only override endpoint dir for the registry, not the smartscopes registry.
    auto endp_dir = (server_name_ == registry_identity_ ? registry_endpoint_dir_ : public_endpoint_dir_);
    string endpoint = "ipc://" + endp_dir + "/" + server_name_ + state_suffix;
    return make_shared<ZmqStateReceiver>(this, endpoint, identity, state_category);
}

MWQueryCtrlProxy ZmqMiddleware::add_query_ctrl_object(QueryCtrlObjectBase::SPtr const& ctrl)
{
    assert(ctrl);

    MWQueryCtrlProxy proxy;
    try
    {
        shared_ptr<QueryCtrlI> qci(make_shared<QueryCtrlI>(ctrl));
        auto adapter = find_adapter(server_name_ + ctrl_suffix, private_endpoint_dir_, ctrl_category);
        function<void()> df;
        auto p = safe_add(df, adapter, "", qci);
        ctrl->set_disconnect_function(df);
        proxy = ZmqQueryCtrlProxy(new ZmqQueryCtrl(this, p->endpoint(), p->identity(), ctrl_category));
        adapter->activate();
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

    try
    {
        shared_ptr<QueryCtrlI> qci(make_shared<QueryCtrlI>(ctrl));
        auto adapter = find_adapter(server_name_ + ctrl_suffix, private_endpoint_dir_, ctrl_category);
        auto df = safe_dflt_add(adapter, ctrl_category, qci);
        ctrl->set_disconnect_function(df);
        adapter->activate();
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
        auto adapter = find_adapter(server_name_ + query_suffix, private_endpoint_dir_, query_category);
        function<void()> df;
        auto p = safe_add(df, adapter, "", qi);
        query->set_disconnect_function(df);
        proxy = ZmqQueryProxy(new ZmqQuery(this, p->endpoint(), p->identity(), query_category));
        adapter->activate();
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_query_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}

void ZmqMiddleware::add_dflt_query_object(QueryObjectBase::SPtr const& query)
{
    assert(query);

    try
    {
        shared_ptr<QueryI> qi(make_shared<QueryI>(query));
        auto adapter = find_adapter(server_name_ + query_suffix, private_endpoint_dir_, query_category);
        auto df = safe_dflt_add(adapter, query_category, qi);
        query->set_disconnect_function(df);
        adapter->activate();
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_dflt_query_object(): " << e.what() << endl;
        throw;
    }
}

MWRegistryProxy ZmqMiddleware::add_registry_object(string const& identity, RegistryObjectBase::SPtr const& registry)
{
    assert(!identity.empty());
    assert(registry);

    MWRegistryProxy proxy;
    try
    {
        shared_ptr<RegistryI> ri(make_shared<RegistryI>(registry));
        auto endp_dir = (server_name_ == registry_identity_ ? registry_endpoint_dir_ : ss_registry_endpoint_dir_);
        auto adapter = find_adapter(server_name_ + registry_suffix, endp_dir, registry_category);
        function<void()> df;
        auto p = safe_add(df, adapter, identity, ri);
        registry->set_disconnect_function(df);
        proxy = ZmqRegistryProxy(new ZmqRegistry(this, p->endpoint(), p->identity(), registry_category, twoway_timeout_));
        adapter->activate();
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
        auto adapter = find_adapter(server_name_ + reply_suffix, public_endpoint_dir_, reply_category);
        function<void()> df;
        auto p = safe_add(df, adapter, "", ri);
        reply->set_disconnect_function(df);
        proxy = ZmqReplyProxy(new ZmqReply(this, p->endpoint(), p->identity(), reply_category));
        adapter->activate();
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_reply_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}

MWScopeProxy ZmqMiddleware::add_scope_object(string const& identity, ScopeObjectBase::SPtr const& scope, int64_t idle_timeout)
{
    assert(!identity.empty());
    assert(scope);

    MWScopeProxy proxy;
    try
    {
        shared_ptr<ScopeI> si(make_shared<ScopeI>(scope));
        auto adapter = find_adapter(server_name_, private_endpoint_dir_, scope_category, idle_timeout);
        function<void()> df;
        auto p = safe_add(df, adapter, identity, si);
        scope->set_disconnect_function(df);
        proxy = ZmqScopeProxy(new ZmqScope(this, p->endpoint(), p->identity(), scope_category, twoway_timeout_));
        adapter->activate();
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

    try
    {
        shared_ptr<ScopeI> si(make_shared<ScopeI>(scope));
        auto adapter = find_adapter(server_name_, private_endpoint_dir_, scope_category);
        auto df = safe_dflt_add(adapter, scope_category, si);
        scope->set_disconnect_function(df);
        adapter->activate();
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_dflt_scope_object(): " << e.what() << endl;
        throw;
    }
}

MWStateReceiverProxy ZmqMiddleware::add_state_receiver_object(std::string const& identity, StateReceiverObject::SPtr const& state_receiver)
{
    assert(!identity.empty());
    assert(state_receiver);

    MWStateReceiverProxy proxy;
    try
    {
        shared_ptr<StateReceiverI> sri(make_shared<StateReceiverI>(state_receiver));
        // Only override endpoint dir for the registry, not the smartscopes registry.
        auto endp_dir = (server_name_ == registry_identity_ ? registry_endpoint_dir_ : public_endpoint_dir_);
        auto adapter = find_adapter(server_name_ + state_suffix, endp_dir, state_category);
        function<void()> df;
        auto p = safe_add(df, adapter, identity, sri);
        state_receiver->set_disconnect_function(df);
        proxy = ZmqStateReceiverProxy(new ZmqStateReceiver(this, p->endpoint(), p->identity(), state_category));
        adapter->activate();
    }
    catch (std::exception const& e) // Should never happen unless our implementation is broken
    {
        // TODO: log this
        cerr << "unexpected exception in add_state_receiver_object(): " << e.what() << endl;
        throw;
    }
    return proxy;
}

MWPublisher::UPtr ZmqMiddleware::create_publisher(std::string const& publisher_id)
{
    // Only override endpoint dir for the registry, not the smartscopes registry.
    auto endp_dir = (server_name_ == registry_identity_ ? registry_endpoint_dir_ : public_endpoint_dir_);
    return MWPublisher::UPtr(new ZmqPublisher(&context_, publisher_id + publisher_suffix, endp_dir));
}

MWSubscriber::UPtr ZmqMiddleware::create_subscriber(std::string const& publisher_id, std::string const& topic)
{
    // Only override endpoint dir for the registry, not the smartscopes registry.
    auto endp_dir = (server_name_ == registry_identity_ ? registry_endpoint_dir_ : public_endpoint_dir_);
    return MWSubscriber::UPtr(new ZmqSubscriber(&context_, publisher_id + publisher_suffix, endp_dir, topic));
}

std::string ZmqMiddleware::get_scope_endpoint()
{
    return "ipc://" + private_endpoint_dir_ + "/" +  server_name_;
}

std::string ZmqMiddleware::get_query_endpoint()
{
    return "inproc://" + server_name_ + query_suffix;
}

std::string ZmqMiddleware::get_query_ctrl_endpoint()
{
    return "ipc://" + private_endpoint_dir_ + "/" +  server_name_ + ctrl_suffix;
}

zmqpp::context* ZmqMiddleware::context() const noexcept
{
    return const_cast<zmqpp::context*>(&context_);
}

ThreadPool* ZmqMiddleware::oneway_pool()
{
    lock(state_mutex_, data_mutex_);
    lock_guard<mutex> state_lock(state_mutex_, std::adopt_lock);
    lock_guard<mutex> invokers_lock(data_mutex_, std::adopt_lock);
    if (state_ != Started)
    {
        throw MiddlewareException("Cannot invoke operations while middleware is stopped");
    }
    return oneway_invoker_.get();
}

ThreadPool* ZmqMiddleware::twoway_pool()
{
    lock(state_mutex_, data_mutex_);
    lock_guard<mutex> state_lock(state_mutex_, std::adopt_lock);
    lock_guard<mutex> invokers_lock(data_mutex_, std::adopt_lock);
    if (state_ != Started)
    {
        throw MiddlewareException("Cannot invoke operations while middleware is stopped");
    }
    return twoway_invokers_.get();
}

int64_t ZmqMiddleware::locate_timeout() const noexcept
{
    return locate_timeout_;
}

int64_t ZmqMiddleware::registry_timeout() const noexcept
{
    return registry_timeout_;
}

ObjectProxy ZmqMiddleware::make_typed_proxy(string const& endpoint,
                                            string const& identity,
                                            string const& category,
                                            RequestMode mode,
                                            int64_t timeout)
{
    // For the time being we only support Scope and Registry types for proxy creation,
    // both of which are twoway interfaces.
    if (mode != RequestMode::Twoway)
    {
        throw MiddlewareException("make_typed_proxy(): cannot create oneway proxies");
    }
    if (category == scope_category)
    {
        auto p = make_shared<ZmqScope>(this, endpoint, identity, category, timeout);
        return ScopeImpl::create(p, runtime(), identity);
    }
    else if (category == registry_category)
    {
        auto p = make_shared<ZmqRegistry>(this, endpoint, identity, category, timeout);
        return make_shared<RegistryImpl>(p, runtime());
    }
    else
    {
        throw MiddlewareException("make_typed_proxy(): unknown category: " + category);
    }
}

shared_ptr<ObjectAdapter> ZmqMiddleware::find_adapter(string const& name, string const& endpoint_dir,
                                                      string const& category, int64_t idle_timeout)
{
    lock(state_mutex_, data_mutex_);
    lock_guard<mutex> state_lock(state_mutex_, std::adopt_lock);
    lock_guard<mutex> map_lock(data_mutex_, std::adopt_lock);

    if (state_ == Stopping || state_ == Stopped)
    {
        throw MiddlewareException("Cannot invoke operations while middleware is stopped");  // TODO: report mw name
    }

    auto it = am_.find(name);
    if (it != am_.end())
    {
        return it->second;
    }

    // We don't have the requested adapter yet, so we create it on the fly.
    int pool_size;
    RequestMode mode;
    if (category == query_category)
    {
        // The query adapter is single or multi-threaded and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        mode = RequestMode::Oneway;
    }
    else if (category == ctrl_category)
    {
        // The ctrl adapter is single-threaded and supports oneway operations only.
        pool_size = 1;
        mode = RequestMode::Oneway;
    }
    else if (category == reply_category)
    {
        // The reply adapter is single- or multi-threaded and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        mode = RequestMode::Oneway;
    }
    else if (category == state_category)
    {
        // The state adapter is single- or multi-threaded and supports oneway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        mode = RequestMode::Oneway;
    }
    else if (category == scope_category)
    {
        // The scope adapter is single- or multi-threaded and supports twoway operations only.
        // TODO: get pool size from config
        pool_size = 1;
        mode = RequestMode::Twoway;
    }
    else if (category == registry_category)
    {
        // The registry adapter is multi-threaded and supports twoway operations only.
        // TODO: get pool size from config
        // NB: On rebind, locate() is called on this adapter. A scope may then call registry methods during
        // its start() method, hence we must ensure this adapter has enough threads available to handle this.
        pool_size = 11;
        mode = RequestMode::Twoway;
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
    if (category == query_category)
    {
        endpoint = "inproc://" + name;
    }
    else
    {
        endpoint = "ipc://" + endpoint_dir + "/" + name;
    }

    auto a = make_shared<ObjectAdapter>(*this, name, endpoint, mode, pool_size, idle_timeout);
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
