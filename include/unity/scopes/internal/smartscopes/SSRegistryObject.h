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
                     uint no_reply_timeout, uint refresh_rate_in_sec, std::string const& sss_url = "",
                     bool caching_enabled = true);
    virtual ~SSRegistryObject() noexcept;

    ScopeMetadata get_metadata(std::string const& scope_id) const override;
    MetadataMap list() const override;

    ScopeProxy locate(std::string const& scope_id) override;

    bool has_scope(std::string const& scope_id) const;
    std::string get_base_url(std::string const& scope_id) const;
    SmartScopesClient::SPtr get_ssclient() const;

private:
    void refresh_thread();

    void get_remote_scopes();
    bool add(RemoteScope const& remotedata, ScopeMetadata const& scope);

private:
    SmartScopesClient::SPtr ssclient_;

    MetadataMap scopes_;
    std::map<std::string, std::string> base_urls_;
    mutable std::mutex scopes_mutex_;

    std::thread refresh_thread_;
    std::mutex refresh_mutex_;
    std::condition_variable_any refresh_cond_;
    bool refresh_stopped_;

    MiddlewareBase::SPtr middleware_;
    std::string ss_scope_endpoint_;
    uint const regular_refresh_timeout_;
    uint next_refresh_timeout_;

    bool caching_enabled_;
};

}  // namespace smartscopes

}  // namespace internal

}  // namespace scopes

}  // namespace unity

#endif  // UNITY_SCOPES_INTERNAL_SMARTSCOPES_SSREGISTRYOBJECT_H
