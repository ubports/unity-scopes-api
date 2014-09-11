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

#ifndef UNITY_SCOPES_INTERNAL_ONLINEACCOUNTCLIENTIMPL_H
#define UNITY_SCOPES_INTERNAL_ONLINEACCOUNTCLIENTIMPL_H

#include <unity/scopes/OnlineAccountClient.h>

#include <libaccounts-glib/accounts-glib.h>
#include <libsignon-glib/signon-glib.h>
#include <unity/util/NonCopyable.h>

#include <condition_variable>
#include <map>
#include <string>
#include <thread>

namespace unity
{

namespace scopes
{

namespace internal
{

struct AccountInfo
{
    OnlineAccountClientImpl* account_client;
    AgAccountId account_id;
    bool service_enabled;
    gulong service_update_signal_id_;
    std::shared_ptr<AgAccountService> account_service;
    std::shared_ptr<SignonAuthSession> session;
    std::shared_ptr<GVariant> auth_params;
    std::shared_ptr<GVariant> session_data;
};

class OnlineAccountClientImpl final
{
public:
    NONCOPYABLE(OnlineAccountClientImpl);

    OnlineAccountClientImpl(std::string const& service_name,
                            std::string const& service_type,
                            std::string const& provider_name,
                            OnlineAccountClient::MainLoopSelect main_loop_select);
    ~OnlineAccountClientImpl();

    void set_service_update_callback(OnlineAccountClient::ServiceUpdateCallback callback);

    std::vector<OnlineAccountClient::ServiceStatus> get_service_statuses();

    void register_account_login_item(Result& result,
                                     CannedQuery const& query,
                                     OnlineAccountClient::PostLoginAction login_passed_action,
                                     OnlineAccountClient::PostLoginAction login_failed_action);

    void register_account_login_item(PreviewWidget& widget,
                                     OnlineAccountClient::PostLoginAction login_passed_action,
                                     OnlineAccountClient::PostLoginAction login_failed_action);

    // Methods used only by impl
    void flush_statuses(std::unique_lock<std::mutex>& lock);

    void main_loop_state_notify(bool is_running);

    std::string service_name();
    void callback(OnlineAccountClient::ServiceStatus const& service_status);

    bool has_account(AgAccountId const& account_id);
    void add_account(AgAccountId const& account_id, std::shared_ptr<AccountInfo> account_info);
    void remove_account(AgAccountId const& account_id);

    bool inc_logins();
    void dec_logins();

private:
    std::string service_name_;
    std::string service_type_;
    std::string provider_name_;
    OnlineAccountClient::ServiceUpdateCallback callback_;
    bool use_external_main_loop_;

    std::thread main_loop_thread_;
    std::mutex callback_mutex_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool main_loop_is_running_;
    bool client_stopping_;
    int logins_busy_;
    gulong account_enabled_signal_id_;
    gulong account_deleted_signal_id_;

    std::shared_ptr<GMainLoop> main_loop_;
    std::shared_ptr<AgManager> manager_;
    std::map<AgAccountId, std::shared_ptr<AccountInfo>> accounts_;

    void main_loop_thread();
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
