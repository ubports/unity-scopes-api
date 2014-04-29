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

#include <unity/scopes/internal/RuntimeConfig.h>

#include <unity/scopes/internal/DfltConfig.h>
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
    const string runtime_config_group = "Runtime";
    const string registry_identity_str = "Registry.Identity";
    const string registry_configfile_str = "Registry.ConfigFile";
    const string default_middleware_str = "Default.Middleware";
    const string default_middleware_configfile_str = "ConfigFile";
}

RuntimeConfig::RuntimeConfig(string const& configfile) :
    ConfigBase(configfile)
{
    if (configfile.empty())  // Default config
    {
        registry_identity_ = DFLT_REGISTRY_ID;
        registry_configfile_ = DFLT_REGISTRY_INI;
        default_middleware_ = "Zmq";
        default_middleware_configfile_ = "Zmq.ini";
    }
    else
    {
        registry_identity_ = get_optional_string(runtime_config_group, registry_identity_str);
        auto pos = registry_identity_.find_first_of("@:/");
        if (pos != string::npos)
        {
            throw_ex("Illegal character in value for " + registry_identity_str + ": \"" + registry_identity_ +
                     "\": identity cannot contain '" + registry_identity_[pos] + "'");
        }
        registry_configfile_ = get_optional_string(runtime_config_group, registry_configfile_str);
        default_middleware_ = get_middleware(runtime_config_group, default_middleware_str);
        default_middleware_configfile_ =
            get_optional_string(runtime_config_group, default_middleware_ + "." + default_middleware_configfile_str);
    }

    const KnownEntries known_entries = {
                                          {  runtime_config_group,
                                             {
                                                registry_identity_str,
                                                registry_configfile_str,
                                                default_middleware_str,
                                                default_middleware_configfile_str
                                             }
                                          }
                                       };
    check_unknown_entries(known_entries);
}

RuntimeConfig::~RuntimeConfig()
{
}

string RuntimeConfig::registry_identity() const
{
    return registry_identity_;
}

string RuntimeConfig::registry_configfile() const
{
    return registry_configfile_;
}

string RuntimeConfig::default_middleware() const
{
    return default_middleware_;
}

string RuntimeConfig::default_middleware_configfile() const
{
    return default_middleware_configfile_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
