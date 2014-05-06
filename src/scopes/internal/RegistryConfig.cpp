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

#include <unity/scopes/internal/RegistryConfig.h>

#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
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
    oem_installdir_ = get_optional_string(REGISTRY_CONFIG_GROUP, "OEM.InstallDir");
    click_installdir_ = get_optional_string(REGISTRY_CONFIG_GROUP, "Click.InstallDir");
    if (click_installdir_.empty())
    {
        char const* home = getenv("HOME");
        if (home && *home != '\0')
        {
            click_installdir_ = string(home) + "/.local/share/unity-scopes/";
        }
    }
    scoperunner_path_ = get_string(REGISTRY_CONFIG_GROUP, "Scoperunner.Path");
    if (scoperunner_path_[0] != '/')
    {
        throw ConfigException(configfile + ": Scoperunner.Path must be an absolute path");
    }
    ss_registry_identity_ = get_optional_string(REGISTRY_CONFIG_GROUP, "SS.Registry.Identity");
    ss_registry_endpoint_ = get_optional_string(REGISTRY_CONFIG_GROUP, "SS.Registry.Endpoint");
}

RegistryConfig::~RegistryConfig()
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

string RegistryConfig::oem_installdir() const
{
    return oem_installdir_;
}

string RegistryConfig::click_installdir() const
{
    return click_installdir_;
}

string RegistryConfig::scoperunner_path() const
{
    return scoperunner_path_;
}

string RegistryConfig::ss_registry_identity() const
{
    return ss_registry_identity_;
}

string RegistryConfig::ss_registry_endpoint() const
{
    return ss_registry_endpoint_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
