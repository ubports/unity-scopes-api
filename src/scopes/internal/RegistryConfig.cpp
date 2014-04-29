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

namespace
{
    const string registry_config_group = "Registry";
    const string mw_kind_str = "Middleware";
    const string endpoint_dir_str = ".EndpointDir";
    const string endpoint_str = ".Endpoint";
    const string configfile_str = ".ConfigFile";
    const string scope_installdir_str = "Scope.InstallDir";
    const string oem_installdir_str = "OEM.InstallDir";
    const string click_installdir_str = "Click.InstallDir";
    const string scoperunner_path_str = "Scoperunner.Path";
    const string ss_registry_identity_str = "SS.Registry.Identity";
    const string ss_registry_endpoint_str = "SS.Registry.Endpoint";
}

RegistryConfig::RegistryConfig(string const& identity, string const& configfile) :
    ConfigBase(configfile)
{
    identity_ = identity;
    if (identity.empty())
    {
        throw InvalidArgumentException("Registry identity cannot be an empty string");
    }
    mw_kind_ = get_middleware(registry_config_group, mw_kind_str);
    string mw_prefix = get_string(registry_config_group, mw_kind_str);
    endpointdir_ = get_string(registry_config_group, mw_kind_ + endpoint_dir_str);
    endpoint_ = get_string(registry_config_group, mw_kind_ + endpoint_str);
    mw_configfile_ = get_optional_string(registry_config_group, mw_kind_ + configfile_str);
    scope_installdir_ = get_string(registry_config_group, scope_installdir_str);
    oem_installdir_ = get_optional_string(registry_config_group, oem_installdir_str);
    click_installdir_ = get_optional_string(registry_config_group, click_installdir_str);
    if (click_installdir_.empty())
    {
        click_installdir_ = string(getenv("HOME")) + "/.local/share/unity-scopes/";
    }
    scoperunner_path_ = get_string(registry_config_group, scoperunner_path_str);
    if (scoperunner_path_[0] != '/')
    {
        throw ConfigException(configfile + ": " + scoperunner_path_str + " must be an absolute path");
    }
    ss_registry_identity_ = get_optional_string(registry_config_group, ss_registry_identity_str);
    ss_registry_endpoint_ = get_optional_string(registry_config_group, ss_registry_endpoint_str);

    const KnownEntries known_entries = {
                                          {  registry_config_group,
                                             {
                                                mw_kind_str,
                                                mw_prefix + endpoint_dir_str,
                                                mw_prefix + endpoint_str,
                                                mw_prefix + configfile_str,
                                                scope_installdir_str,
                                                oem_installdir_str,
                                                click_installdir_str,
                                                scoperunner_path_str,
                                                ss_registry_identity_str,
                                                ss_registry_endpoint_str
                                             }
                                          }
                                       };
    check_unknown_entries(known_entries);
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
