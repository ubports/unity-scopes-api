/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
*/

#include <unity/scopes/internal/OnlineAccountClientImpl.h>

#include <unity/UnityExceptions.h>

#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"  // Lots of warnings from libg otherwise.

namespace unity
{

namespace scopes
{

namespace internal
{

static void free_error(GError* e)
{
    if (e)
    {
        g_error_free(e);
    }
}

static void free_variant(GVariant* v)
{
    if (v)
    {
        g_variant_unref(v);
    }
}

static OnlineAccountClient::ServiceStatus info_to_details(AccountInfo* info, std::string const& error = "")
{
    char* client_id = nullptr;
    char* client_secret = nullptr;
    char* access_token = nullptr;
    char* token_secret = nullptr;

    std::lock_guard<std::mutex> info_lock(info->mutex);

    if (info->auth_params)
    {
        // Look up OAuth 2 parameters
        g_variant_lookup(info->auth_params.get(), "ClientId", "&s", &client_id);
        g_variant_lookup(info->auth_params.get(), "ClientSecret", "&s", &client_secret);
        // If OAuth 2 parameters are not found, fall back to OAuth 1 parameters
        if (!client_id)
        {
            g_variant_lookup(info->auth_params.get(), "ConsumerKey", "&s", &client_id);
        }
        if (!client_secret)
        {
            g_variant_lookup(info->auth_params.get(), "ConsumerSecret", "&s", &client_secret);
        }
    }
    if (info->session_data)
    {
        g_variant_lookup(info->session_data.get(), "AccessToken", "&s", &access_token);
        g_variant_lookup(info->session_data.get(), "TokenSecret", "&s", &token_secret);
    }

    // Gather the latest service details then trigger the client callback
    OnlineAccountClient::ServiceStatus service_status;
    service_status.account_id = info->account_id;
    service_status.service_enabled = info->service_enabled;
    service_status.service_authenticated = access_token ? info->service_enabled : false;
    service_status.client_id = client_id ? client_id : "";
    service_status.client_secret = client_secret ? client_secret : "";
    service_status.access_token = (info->service_enabled && access_token) ? access_token : "";
    service_status.token_secret = (info->service_enabled && token_secret) ? token_secret : "";
    service_status.error = error;

    return service_status;
}

static gboolean main_loop_is_running_cb(void* user_data)
{
    OnlineAccountClientImpl* account_client = reinterpret_cast<OnlineAccountClientImpl*>(user_data);
    account_client->main_loop_state_notify(true);
    return G_SOURCE_REMOVE;
}

static gboolean main_loop_is_stopping_cb(void* user_data)
{
    OnlineAccountClientImpl* account_client = reinterpret_cast<OnlineAccountClientImpl*>(user_data);
    account_client->main_loop_state_notify(false);
    return G_SOURCE_REMOVE;
}

static gboolean wake_up_event_loop_cb(void* user_data)
{
    GMainLoop* event_loop = reinterpret_cast<GMainLoop*>(user_data);
    g_main_loop_quit(event_loop);
    return G_SOURCE_REMOVE;
}

static void service_login_cb(GObject* source, GAsyncResult* result, void* user_data)
{
    SignonAuthSession* session = reinterpret_cast<SignonAuthSession*>(source);
    AccountInfo* info = reinterpret_cast<AccountInfo*>(user_data);

    std::unique_lock<std::mutex> info_lock(info->mutex);

    // Get session data then send a notification with the login result
    GError* error = nullptr;
    info->session_data.reset(signon_auth_session_process_finish(session, result, &error), free_variant);
    std::shared_ptr<GError> error_cleanup(error, free_error);

    info_lock.unlock();
    info->account_client->callback(info, error ? error->message : "");
}

static GVariant *variant_to_gvariant(const Variant &v)
{
    switch (v.which()) {
    case Variant::Int: return g_variant_new_int32(v.get_int());
    case Variant::Bool: return g_variant_new_boolean(v.get_bool());
    case Variant::String: return g_variant_new_string(v.get_string().c_str());
    case Variant::Double: return g_variant_new_double(v.get_double());
    default:
        std::cerr << "variant_to_gvariant(): unsupported type " << v.serialize_json();
        return nullptr;
    }
}

static void service_update_cb(AgAccountService* account_service, gboolean enabled, AccountInfo* info)
{
    std::unique_lock<std::mutex> info_lock(info->mutex);

    // If another session is currently busy, cancel it
    if (info->session)
    {
        signon_auth_session_cancel(info->session.get());
    }

    // Service state has updated, clear the old session data
    info->service_enabled = enabled;

    if (enabled)
    {
        // Get authorization data then create a new authorization session
        std::shared_ptr<AgAuthData> auth_data(ag_account_service_get_auth_data(account_service), ag_auth_data_unref);

        GError* error = nullptr;
        info->session.reset(signon_auth_session_new(
            ag_auth_data_get_credentials_id(auth_data.get()), ag_auth_data_get_method(auth_data.get()), &error), g_object_unref);
        std::shared_ptr<GError> error_cleanup(error, free_error);

        if (error)
        {
            // Send notification that the authorization session failed
            info_lock.unlock();                                    // LCOV_EXCL_LINE
            info->account_client->callback(info, error->message);  // LCOV_EXCL_LINE
            return;
        }

        // Get authorization parameters then attempt to signon
        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE_VARDICT);

        if (getenv("UNITY_SCOPES_OA_UI_POLICY"))
        {
            g_variant_builder_add(&builder, "{sv}",
                                  SIGNON_SESSION_DATA_UI_POLICY,
                                  g_variant_new_int32(SIGNON_POLICY_DEFAULT));  // LCOV_EXCL_LINE
        }
        else
        {
            g_variant_builder_add(&builder, "{sv}",
                                  SIGNON_SESSION_DATA_UI_POLICY,
                                  g_variant_new_int32(SIGNON_POLICY_NO_USER_INTERACTION));
        }

        for (const auto &param: info->account_client->auth_params())
        {
            GVariant* value = variant_to_gvariant(param.second);
            if (!value) continue;
            g_variant_builder_add(&builder, "{sv}", param.first.c_str(), value);
        }

        info->auth_params.reset(
            g_variant_ref_sink(ag_auth_data_get_login_parameters(auth_data.get(), g_variant_builder_end(&builder))), free_variant);

        // Start signon process
        signon_auth_session_process(info->session.get(),
                                          info->auth_params.get(),
                                          ag_auth_data_get_mechanism(auth_data.get()),
                                          nullptr,
                                          service_login_cb,
                                          info);
    }
    else
    {
        // Send notification that account has been disabled
        info_lock.unlock();
        info->account_client->callback(info);
    }
}

static void account_enabled_cb(AgManager* manager, AgAccountId account_id, OnlineAccountClientImpl* account_client)
{
    if (account_client->has_account(account_id))
    {
        // We are already watching this account
        return;
    }
    std::shared_ptr<AgAccount> account(ag_manager_get_account(manager, account_id), g_object_unref);
    if (!account)
    {
        // The account was not found
        return;  // LCOV_EXCL_LINE
    }
    // Find the service we're concerned with
    std::shared_ptr<GList> services(ag_account_list_services(account.get()), ag_service_list_free);
    GList* it;
    for (it = services.get(); it; it = it->next)
    {
        AgService* service = reinterpret_cast<AgService*>(it->data);
        if (account_client->service_name() == ag_service_get_name(service))
        {
            std::shared_ptr<AgAccountService> account_service(ag_account_service_new(account.get(), service), g_object_unref);
            std::shared_ptr<AccountInfo> info(new AccountInfo);
            info->service_enabled = false;
            info->account_client = account_client;
            info->account_service.reset(reinterpret_cast<AgAccountService*>(g_object_ref(account_service.get())), g_object_unref);

            AgAccount* account = ag_account_service_get_account(account_service.get());
            g_object_get(account, "id", &info->account_id, nullptr);

            // Watch for changes to this service
            info->service_update_signal_id_ = g_signal_connect(account_service.get(), "enabled", G_CALLBACK(service_update_cb), info.get());

            // Set initial state
            service_update_cb(account_service.get(), ag_account_service_get_enabled(account_service.get()), info.get());
            account_client->add_account(account_id, info);
            break;
        }
    }
}

static void account_deleted_cb(AgManager*, AgAccountId account_id, OnlineAccountClientImpl* account_client)
{
    // A disabled event should have been sent prior to this, so no
    // need to send any notification.
    account_client->remove_account(account_id);
}

OnlineAccountClientImpl::OnlineAccountClientImpl(std::string const& service_name,
                                                 std::string const& service_type,
                                                 std::string const& provider_name,
                                                 VariantMap const& auth_params,
                                                 OnlineAccountClient::MainLoopSelect main_loop_select)
    : service_name_(service_name)
    , service_type_(service_type)
    , provider_name_(provider_name)
    , auth_params_(auth_params)
    , main_loop_select_(main_loop_select)
    , main_loop_is_running_(main_loop_select != OnlineAccountClient::CreateInternalMainLoop)
{
    // If we are responsible for the main loop
    if (main_loop_select_ == OnlineAccountClient::CreateInternalMainLoop)
    {
        // Wait here until either the main loop begins running, or the thread exits
        std::unique_lock<std::mutex> lock(mutex_);
        main_loop_thread_ = std::thread(&OnlineAccountClientImpl::main_loop_thread, this);
        cond_.wait_for(lock, std::chrono::seconds(5), [this] { return main_loop_is_running_; });

        if (!main_loop_is_running_)
        {
            // LCOV_EXCL_START
            if (main_loop_)
            {
                // Quit the main loop, causing the thread to exit
                g_main_loop_quit(main_loop_.get());
            }
            if (main_loop_thread_.joinable())
            {
                lock.unlock();
                main_loop_thread_.join();
                lock.lock();
            }
            if (thread_exception_)
            {
                std::rethrow_exception(thread_exception_);
            }
            else
            {
                throw unity::ResourceException("OnlineAccountClientImpl(): main_loop_thread failed to start.");
            }
            // LCOV_EXCL_STOP
        }
    }
    else
    {
        construct();
    }
}

OnlineAccountClientImpl::~OnlineAccountClientImpl()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (thread_exception_)
        {
            // TODO: We don't have access to the run time here, so we can't write to the log.
            //       It would be better to have a factory method on Runtime that instantiates
            //       an OnlineAccoutClient, in which case we could access the logger here.
            //       But that's an ABI-breaking change...
            // LCOV_EXCL_START
            try
            {
                std::rethrow_exception(thread_exception_);
            }
            catch (std::exception const& e)
            {
                std::cerr << "~OnlineAccountClientImpl(): main_loop_thread threw an exception: " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "~OnlineAccountClientImpl(): main_loop_thread threw an unknown exception" << std::endl;
            }
            // LCOV_EXCL_STOP
        }
    }

    // If we are responsible for the main loop
    if (main_loop_select_ == OnlineAccountClient::CreateInternalMainLoop)
    {
        // Invoke tear_down() from within the main loop
        std::unique_lock<std::mutex> lock(mutex_);
        g_main_context_invoke(main_loop_context_.get(), main_loop_is_stopping_cb, this);
        cond_.wait(lock, [this] { return !main_loop_is_running_; });

        if (main_loop_)
        {
            // Quit the main loop, causing the thread to exit
            g_main_loop_quit(main_loop_.get());
        }
        if (main_loop_thread_.joinable())
        {
            lock.unlock();
            main_loop_thread_.join();
            lock.lock();
        }
    }
    else
    {
        tear_down();
    }

    std::unique_lock<std::mutex> lock(callback_mutex_);
    if (callback_thread_.joinable())
    {
        lock.unlock();
        callback_thread_.join();
        lock.lock();
    }
}

void OnlineAccountClientImpl::set_service_update_callback(OnlineAccountClient::ServiceUpdateCallback callback)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (thread_exception_)
    {
        std::rethrow_exception(thread_exception_);  // LCOV_EXCL_LINE
    }

    std::lock_guard<std::mutex> callback_lock(callback_mutex_);
    callback_ = callback;

    if (callback_ != nullptr)
    {
        std::vector<OnlineAccountClient::ServiceStatus> known_statuses;
        for (auto const& info : accounts_)
        {
            // We only want to invoke the callback for non-busy sessions, as signon sessions that are currently
            // busy will end with a callback anyway. info->session is cleared once a signon process has fully
            // completed, therefore, if the session is null, we know that it is not currently busy.
            std::unique_lock<std::mutex> info_lock(info.second->mutex);
            if (info.second->session == nullptr)
            {
                info_lock.unlock();
                known_statuses.push_back(info_to_details(info.second.get()));
            }
        }

        // Invoke callback for all known service statuses
        lock.unlock();
        for (auto const& status : known_statuses)
        {
            callback_(status);
        }
    }
}

void OnlineAccountClientImpl::refresh_service_statuses()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (thread_exception_)
    {
        std::rethrow_exception(thread_exception_);  // LCOV_EXCL_LINE
    }

    // Update the accounts we already know about
    for (auto const& info : accounts_)
    {
        service_update_cb(info.second->account_service.get(), ag_account_service_get_enabled(info.second->account_service.get()), info.second.get());
    }

    // Find new account we don't yet know about
    std::shared_ptr<GList> enabled_accounts(ag_manager_list(manager_.get()), ag_manager_list_free);
    GList* it;
    for (it = enabled_accounts.get(); it; it = it->next)
    {
        AgAccountId account_id = GPOINTER_TO_UINT(it->data);
        std::shared_ptr<AgAccount> account(ag_manager_get_account(manager_.get(), account_id), g_object_unref);
        std::string provider_name = ag_account_get_provider_name(account.get());
        if (provider_name == provider_name_)
        {
            lock.unlock();
            account_enabled_cb(manager_.get(), account_id, this);
            lock.lock();
        }
    }
}

std::vector<OnlineAccountClient::ServiceStatus> OnlineAccountClientImpl::get_service_statuses()
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (thread_exception_)
    {
        std::rethrow_exception(thread_exception_);  // LCOV_EXCL_LINE
    }

    // Return all service statuses
    std::vector<OnlineAccountClient::ServiceStatus> service_statuses;
    for (auto const& info : accounts_)
    {
        flush_pending_session(info.second.get(), lock);
        service_statuses.push_back(info_to_details(info.second.get()));
    }
    return service_statuses;
}

void OnlineAccountClientImpl::register_account_login_item(Result& result,
                                                          CannedQuery const& query,
                                                          OnlineAccountClient::PostLoginAction login_passed_action,
                                                          OnlineAccountClient::PostLoginAction login_failed_action)
{
    // If no URI is set on this result, default to a canned query to refresh the scope results
    if (result.uri().empty())
    {
        const unity::scopes::CannedQuery refresh_query(query.scope_id(), query.query_string(), query.query_string().empty() ? query.department_id() : "");
        result.set_uri(refresh_query.to_uri());
    }

    VariantMap account_details_map;
    account_details_map["scope_id"] = query.scope_id();
    account_details_map["service_name"] = service_name_;
    account_details_map["service_type"] = service_type_;
    account_details_map["provider_name"] = provider_name_;
    account_details_map["auth_params"] = auth_params_;
    account_details_map["login_passed_action"] = static_cast<int>(login_passed_action);
    account_details_map["login_failed_action"] = static_cast<int>(login_failed_action);

    result["online_account_details"] = Variant(account_details_map);
}

void OnlineAccountClientImpl::register_account_login_item(PreviewWidget& widget,
                                                          OnlineAccountClient::PostLoginAction login_passed_action,
                                                          OnlineAccountClient::PostLoginAction login_failed_action)
{
    VariantMap account_details_map;
    account_details_map["scope_id"] = "";
    account_details_map["service_name"] = service_name_;
    account_details_map["service_type"] = service_type_;
    account_details_map["provider_name"] = provider_name_;
    account_details_map["auth_params"] = auth_params_;
    account_details_map["login_passed_action"] = static_cast<int>(login_passed_action);
    account_details_map["login_failed_action"] = static_cast<int>(login_failed_action);

    widget.add_attribute_value("online_account_details", Variant(account_details_map));
}

void OnlineAccountClientImpl::construct()
{
    manager_.reset(ag_manager_new_for_service_type(service_type_.c_str()), g_object_unref);

    // Watch for changes to accounts
    account_enabled_signal_id_ = g_signal_connect(manager_.get(), "enabled-event", G_CALLBACK(account_enabled_cb), this);
    account_deleted_signal_id_ = g_signal_connect(manager_.get(), "account-deleted", G_CALLBACK(account_deleted_cb), this);

    // Now check initial state
    refresh_service_statuses();
}

void OnlineAccountClientImpl::tear_down()
{
    // Disconnect signal handlers
    g_signal_handler_disconnect(manager_.get(), account_enabled_signal_id_);
    g_signal_handler_disconnect(manager_.get(), account_deleted_signal_id_);

    // Remove all accounts
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (auto const& info : accounts_)
        {
            // Before we nuke the map, ensure that any pending sessions are done
            {
                std::lock_guard<std::mutex> info_lock(info.second->mutex);
                g_signal_handler_disconnect(info.second->account_service.get(), info.second->service_update_signal_id_);
            }
            flush_pending_session(info.second.get(), lock);
        }
        accounts_.clear();
    }
}

void OnlineAccountClientImpl::flush_pending_session(AccountInfo* info, std::unique_lock<std::mutex>& lock)
{
    std::unique_lock<std::mutex> info_lock(info->mutex);

    // Wait until all currently running login sessions are done
    // (ensures that accounts_ is up to date)
    std::shared_ptr<GMainLoop> event_loop;
    event_loop.reset(g_main_loop_new(main_loop_context_.get(), true), g_main_loop_unref);

    int ms_elapsed = 0;
    while (info->session && ms_elapsed < 5000)
    {
        // We need to wait inside an event loop to allow for the main application loop to
        // process its pending events
        std::shared_ptr<GSource> source;
        source.reset(g_timeout_source_new(10), g_source_unref);
        ms_elapsed += 10;
        g_source_set_callback(source.get(), wake_up_event_loop_cb, event_loop.get(), NULL);
        g_source_attach(source.get(), main_loop_context_.get());

        info_lock.unlock();
        lock.unlock();
        g_main_loop_run(event_loop.get());
        lock.lock();
        info_lock.lock();
    }

    // If the login session is still not responding, we cancel it
    if (info->session)
    {
        std::cerr << "OnlineAccountClientImpl::flush_pending_session(): login for account: "
                  << info->account_id << " is not responding. Canceling session." << std::endl;
        signon_auth_session_cancel(info->session.get());
        info->session = nullptr;
    }
}

void OnlineAccountClientImpl::main_loop_state_notify(bool is_running)
{
    bool was_running = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        was_running = main_loop_is_running_;
    }
    if (!was_running && is_running)
    {
        construct();
    }
    else if (was_running && !is_running)
    {
        tear_down();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    main_loop_is_running_ = is_running;
    cond_.notify_all();
}

std::shared_ptr<AgManager> OnlineAccountClientImpl::manager()
{
    return manager_;
}

std::string OnlineAccountClientImpl::service_name()
{
    return service_name_;
}

std::shared_ptr<GMainContext> OnlineAccountClientImpl::main_loop_context()
{
    return main_loop_context_;
}

void OnlineAccountClientImpl::callback(AccountInfo* info, std::string const& error)
{
    std::unique_lock<std::mutex> lock(callback_mutex_);
    if (callback_thread_.joinable())
    {
        lock.unlock();
        callback_thread_.join();
        lock.lock();
    }

    OnlineAccountClient::ServiceStatus status = info_to_details(info, error);
    callback_thread_ = std::thread([this, status]
    {
        std::lock_guard<std::mutex> lock(callback_mutex_);
        if (callback_)
        {
            callback_(status);
        }
    });

    // Clear session info
    std::lock_guard<std::mutex> info_lock(info->mutex);
    info->session = nullptr;
}

bool OnlineAccountClientImpl::has_account(AgAccountId const& account_id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return accounts_.find(account_id) != accounts_.end();
}

void OnlineAccountClientImpl::add_account(AgAccountId const& account_id, std::shared_ptr<AccountInfo> account_info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    accounts_.insert(std::make_pair(account_id, account_info));
}

void OnlineAccountClientImpl::remove_account(AgAccountId const& account_id)
{
    std::unique_lock<std::mutex> lock(mutex_);

    // Get account info
    auto info_it = accounts_.find(account_id);
    if (info_it == accounts_.end())
    {
        return;
    }
    auto info = info_it->second;

    // Before we nuke the pointer, ensure that any pending sessions are done
    {
        std::lock_guard<std::mutex> info_lock(info->mutex);
        g_signal_handler_disconnect(info->account_service.get(), info->service_update_signal_id_);
    }
    flush_pending_session(info.get(), lock);

    // Remove account info from accounts_ map
    accounts_.erase(account_id);
}

void OnlineAccountClientImpl::main_loop_thread()
{
    // If something goes wrong causing the thread to abort, the destruction of this pointer update the
    // main_loop_is_running_ state accordingly.
    std::shared_ptr<void> thread_exit_notifier(nullptr, [this](void*){ main_loop_state_notify(false); });

    try
    {
        main_loop_context_.reset(g_main_context_new(), g_main_context_unref);
        g_main_context_push_thread_default(main_loop_context_.get());

        // Stick a method call into the main loop to notify the constructor when the main loop begins running
        g_main_context_invoke(main_loop_context_.get(), main_loop_is_running_cb, this);

        // Run the main loop
        main_loop_.reset(g_main_loop_new(main_loop_context_.get(), true), g_main_loop_unref);
        g_main_loop_run(main_loop_.get());
    }
    // LCOV_EXCL_START
    catch (std::exception const&)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_exception_ = std::current_exception();
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_exception_ = std::current_exception();
    }
    // LCOV_EXCL_STOP
}

#pragma GCC diagnostic pop

}  // namespace internal

}  // namespace scopes

}  // namespace unity
