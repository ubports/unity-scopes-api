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

/**
 \brief blah
*/

class OnlineAccountClient final
{
public:
    /// @cond
    NONCOPYABLE(OnlineAccountClient);
    /// @endcond

    /**
    \brief blah
    */
    struct ServiceStatus
    {
        uint account_id;            ///< blah
        bool service_enabled;       ///< blah
        std::string client_id;      ///< blah
        std::string client_secret;  ///< blah
        std::string access_token;   ///< blah
        std::string token_secret;   ///< blah
        std::string error;          //!< blah
    };

    /**
    \brief blah
    */
    typedef std::function<void(ServiceStatus const&)> ServiceUpdateCallback;

    /**
    \brief blah
    */
    enum MainLoopSelect
    {
        RunInExternalMainLoop,  ///< blah
        CreateInternalMainLoop  ///< blah
    };

    /**
    \brief blah
    \param service_name blah
    \param service_type blah
    \param provider_name blah
    \param main_loop_select blah
    */
    OnlineAccountClient(std::string const& service_name,
                        std::string const& service_type,
                        std::string const& provider_name,
                        MainLoopSelect main_loop_select = RunInExternalMainLoop);

    /// @cond
    ~OnlineAccountClient();
    /// @endcond

    /**
    \brief blah
    \param callback blah
    */
    void set_service_update_callback(ServiceUpdateCallback callback);

    /**
    \brief blah
    */
    void refresh_service_statuses();

    /**
    \brief blah
    \return blah
    */
    std::vector<ServiceStatus> get_service_statuses();

    /**
    \brief blah
    */
    enum PostLoginAction
    {
        Unknown,                                ///< blah
        DoNothing,                              ///< blah
        InvalidateResults,                      ///< blah
        ContinueActivation,                     ///< blah
        LastActionCode_ = ContinueActivation    ///< blah
    };

    /**
    \brief blah
    \param result blah
    \param query blah
    \param login_passed_action blah
    \param login_failed_action blah
    */
    void register_account_login_item(Result& result,
                                     CannedQuery const& query,
                                     PostLoginAction login_passed_action,
                                     PostLoginAction login_failed_action);

    /**
    \brief blah
    \param widget blah
    \param login_passed_action blah
    \param login_failed_action blah
    */
    void register_account_login_item(PreviewWidget& widget,
                                     PostLoginAction login_passed_action,
                                     PostLoginAction login_failed_action);

private:
    std::unique_ptr<internal::OnlineAccountClientImpl> p;
};

} // namespace scopes

} // namespace unity

#endif
