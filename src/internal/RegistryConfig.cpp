/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/internal/RegistryConfig.h>

#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

constexpr char const* RegistryConfig::REGISTRY_CONFIG_GROUP;

RegistryConfig::RegistryConfig(string const& identity, string const& configfile) :
    ConfigBase(configfile)
{
    identity_ = identity;
    if (identity.empty())
    {
        throw InvalidArgumentException("Registry identity cannot be an empty string");
    }
    mw_kind_ = get_middleware(REGISTRY_CONFIG_GROUP, "Middleware");
    endpointdir_ = get_string(REGISTRY_CONFIG_GROUP, mw_kind_ + ".EndpointDir");
    endpoint_ = get_string(REGISTRY_CONFIG_GROUP, mw_kind_ + ".Endpoint");
    mw_configfile_ = get_string(REGISTRY_CONFIG_GROUP, mw_kind_ + ".ConfigFile");
    scope_installdir_ = get_string(REGISTRY_CONFIG_GROUP, "Scope.InstallDir");
    scope_group_configdir_ = get_optional_string(REGISTRY_CONFIG_GROUP, "Scope.GroupConfigDir");
    oem_installdir_ = get_optional_string(REGISTRY_CONFIG_GROUP, "OEM.InstallDir");
    oem_group_configdir_ = get_optional_string(REGISTRY_CONFIG_GROUP, "OEM.GroupConfigDir");
    scoperunner_path_ = get_string(REGISTRY_CONFIG_GROUP, "Scoperunner.Path");
    if (scoperunner_path_[0] != '/')
    {
        throw ConfigException(configfile + ": Scoperunner.Path must be an absolute path");
    }
}

RegistryConfig::~RegistryConfig() noexcept
{
}

string RegistryConfig::identity() const
{
    return identity_;
}

string RegistryConfig::mw_kind() const
{
    return mw_kind_;
}

string RegistryConfig::endpointdir() const
{
    return endpointdir_;
}

string RegistryConfig::endpoint() const
{
    return endpoint_;
}

string RegistryConfig::mw_configfile() const
{
    return mw_configfile_;
}

string RegistryConfig::scope_installdir() const
{
    return scope_installdir_;
}

string RegistryConfig::scope_group_configdir() const
{
    return scope_group_configdir_;
}

string RegistryConfig::oem_installdir() const
{
    return oem_installdir_;
}

string RegistryConfig::oem_group_configdir() const
{
    return oem_group_configdir_;
}

string RegistryConfig::scoperunner_path() const
{
    return scoperunner_path_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
