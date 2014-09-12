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

#include <unity/scopes/OnlineAccountClient.h>
#include <unity/scopes/internal/OnlineAccountClientImpl.h>

namespace unity
{

namespace scopes
{

OnlineAccountClient::OnlineAccountClient(std::string const& service_name,
                                         std::string const& service_type,
                                         std::string const& provider_name,
                                         MainLoopSelect main_loop_select)
    : p(new internal::OnlineAccountClientImpl(service_name, service_type, provider_name, main_loop_select))
{
}

/// @cond
OnlineAccountClient::~OnlineAccountClient() = default;
/// @endcond

void OnlineAccountClient::set_service_update_callback(ServiceUpdateCallback callback)
{
    p->set_service_update_callback(callback);
}

void OnlineAccountClient::refresh_service_statuses()
{
    p->refresh_service_statuses();
}

std::vector<OnlineAccountClient::ServiceStatus> OnlineAccountClient::get_service_statuses()
{
    return p->get_service_statuses();
}

void OnlineAccountClient::register_account_login_item(Result& result,
                                                      CannedQuery const& query,
                                                      PostLoginAction login_passed_action,
                                                      PostLoginAction login_failed_action)
{
    p->register_account_login_item(result, query, login_passed_action, login_failed_action);
}

void OnlineAccountClient::register_account_login_item(PreviewWidget& widget,
                                                      PostLoginAction login_passed_action,
                                                      PostLoginAction login_failed_action)
{
    p->register_account_login_item(widget, login_passed_action, login_failed_action);
}

} // namespace scopes

} // namespace unity
