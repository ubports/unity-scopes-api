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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSREGISTRYOBJECT_H
#define UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSREGISTRYOBJECT_H

#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/RegistryObjectBase.h>
#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>

#include <mutex>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace smartscopes
{

class SSRegistryObject final : public RegistryObjectBase
{
public:
    UNITY_DEFINES_PTRS(SSRegistryObject);

    SSRegistryObject(MiddlewareBase::SPtr middleware, std::string const& ss_scope_endpoint,
                     uint max_http_sessions, uint no_reply_timeout, uint refresh_rate_in_min);
    virtual ~SSRegistryObject() noexcept;

    ScopeMetadata get_metadata(std::string const& scope_name) override;
    MetadataMap list() override;

    ScopeProxy locate(std::string const& scope_name) override;

    std::string get_base_url(std::string const& scope_name);
    SmartScopesClient::SPtr get_ssclient();

private:
    void refresh_thread();

    void get_remote_scopes();
    bool add(std::string const& scope_name, ScopeMetadata const& scope, RemoteScope const& remotedata);

private:
    SmartScopesClient::SPtr ssclient_;

    MetadataMap scopes_;
    std::map<std::string, std::string> base_urls_;
    std::mutex scopes_mutex_;

    std::thread refresh_thread_;
    std::mutex refresh_mutex_;
    std::condition_variable_any refresh_cond_;
    bool refresh_stopped_;

    MiddlewareBase::SPtr middleware_;
    std::string ss_scope_endpoint_;
    uint const refresh_rate_in_min_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSREGISTRYOBJECT_H
