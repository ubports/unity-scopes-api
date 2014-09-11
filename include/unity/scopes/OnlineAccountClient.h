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

#ifndef UNITY_SCOPES_ONLINEACCOUNTCLIENT_H
#define UNITY_SCOPES_ONLINEACCOUNTCLIENT_H

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>
#include <unity/util/NonCopyable.h>

#include <memory>

namespace unity
{

namespace scopes
{

namespace internal
{
class OnlineAccountClientImpl;
}

class OnlineAccountClient final
{
public:
    /// @cond
    NONCOPYABLE(OnlineAccountClient);
    /// @endcond

    struct ServiceStatus
    {
        uint account_id;
        bool service_enabled;
        std::string client_id;
        std::string client_secret;
        std::string access_token;
        std::string token_secret;
        std::string error;
    };

    typedef std::function<void(ServiceStatus const&)> ServiceUpdateCallback;

    enum MainLoopSelect
    {
        RunInExternalMainLoop,
        CreateInternalMainLoop
    };

    OnlineAccountClient(std::string const& service_name,
                        std::string const& service_type,
                        std::string const& provider_name,
                        MainLoopSelect main_loop_select = RunInExternalMainLoop);
    ~OnlineAccountClient();

    void set_service_update_callback(ServiceUpdateCallback callback);

    void refresh_service_statuses();

    std::vector<ServiceStatus> get_service_statuses();

    enum PostLoginAction
    {
        Unknown,
        DoNothing,
        InvalidateResults,
        ContinueActivation,
        LastActionCode_ = ContinueActivation // Dummy end marker
    };

    void register_account_login_item(Result& result,
                                     CannedQuery const& query,
                                     PostLoginAction login_passed_action,
                                     PostLoginAction login_failed_action);

    void register_account_login_item(PreviewWidget& widget,
                                     PostLoginAction login_passed_action,
                                     PostLoginAction login_failed_action);

private:
    std::unique_ptr<internal::OnlineAccountClientImpl> p;
};

} // namespace scopes

} // namespace unity

#endif
