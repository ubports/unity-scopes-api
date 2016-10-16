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

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <stdlib.h>

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
    const string mw_kind_key = "Middleware";
    const string configfile_key = ".ConfigFile";
    const string scope_installdir_key = "Scope.InstallDir";
    const string oem_installdir_key = "OEM.InstallDir";
    const string click_installdir_key = "Click.InstallDir";
    const string scoperunner_path_key = "Scoperunner.Path";
    const string process_timeout_key = "Process.Timeout";
}

RegistryConfig::RegistryConfig(string const& identity, string const& configfile) :
    ConfigBase(configfile, DFLT_REGISTRY_INI)
{
    identity_ = identity;
    mw_kind_ = get_middleware(registry_config_group, mw_kind_key);
    mw_configfile_ = get_optional_string(registry_config_group, mw_kind_ + configfile_key);
    scope_installdir_ = get_optional_string(registry_config_group, scope_installdir_key, snap_root() + DFLT_SCOPE_INSTALL_DIR);
    oem_installdir_ = get_optional_string(registry_config_group, oem_installdir_key, DFLT_OEM_INSTALL_DIR);
    click_installdir_ = get_optional_string(registry_config_group, click_installdir_key);
    if (click_installdir_.empty())
    {
        char const* home = getenv("HOME");
        if (!home || *home == '\0')
        {
            throw ConfigException(configfile + ": No Click.InstallDir configured and $HOME not set");
        }
        click_installdir_ = string(home) + "/.local/share/unity-scopes/";
    }
    scoperunner_path_ = get_optional_string(registry_config_group, scoperunner_path_key, snap_root() + DFLT_SCOPERUNNER_PATH);
    if (scoperunner_path_[0] != '/')
    {
        throw ConfigException(configfile + ": " + scoperunner_path_key + " must be an absolute path");
    }
    process_timeout_ = get_optional_int(registry_config_group, process_timeout_key, DFLT_PROCESS_TIMEOUT);
    if (process_timeout_ < 10 || process_timeout_ > 60000)
    {
        throw_ex("Illegal value (" + to_string(process_timeout_) + ") for " + process_timeout_key + ": value must be 10-60000 ms");
    }

    KnownEntries const known_entries = {
                                          {  registry_config_group,
                                             {
                                                mw_kind_key,
                                                mw_kind_ + configfile_key,
                                                scope_installdir_key,
                                                oem_installdir_key,
                                                click_installdir_key,
                                                scoperunner_path_key,
                                                process_timeout_key
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

int RegistryConfig::process_timeout() const
{
    return process_timeout_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
