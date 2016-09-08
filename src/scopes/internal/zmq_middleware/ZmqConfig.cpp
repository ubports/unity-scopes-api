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

#include <unity/scopes/internal/zmq_middleware/ZmqConfig.h>

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/ScopeExceptions.h>

#include <stdlib.h>
#include <unistd.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{
    const string zmq_config_group = "Zmq";
    const string endpoint_dir_key = "EndpointDir";
    const string twoway_timeout_key = "Default.Twoway.Timeout";
    const string locate_timeout_key = "Locate.Timeout";
    const string registry_timeout_key = "Registry.Timeout";
    const string child_scopes_timeout_key = "ChildScopes.Timeout";
    const string registry_endpoint_dir_key = "Registry.EndpointDir";
    const string ss_registry_endpoint_dir_key = "Smartscopes.Registry.EndpointDir";
}

ZmqConfig::ZmqConfig(string const& configfile) :
    ConfigBase(configfile, DFLT_ZMQ_MIDDLEWARE_INI)
{
    if (!configfile.empty())
    {
        endpoint_dir_ = get_optional_string(zmq_config_group, endpoint_dir_key);
    }

    // Set the endpoint directory if it was not set explicitly.
    // We look for the XDG_RUNTIME_DIR env variable. If that is not
    // set correctly, we give up.
    if (endpoint_dir_.empty())
    {
        char* xdg_runtime_dir = secure_getenv("XDG_RUNTIME_DIR");
        if (!xdg_runtime_dir || *xdg_runtime_dir == '\0')
        {
            throw ConfigException("No endpoint directories specified, and XDG_RUNTIME_DIR "
                                  "environment variable not set");
        }
        if (*xdg_runtime_dir != '/')
        {
            throw ConfigException("Invalid XDG_RUNTIME_DIR: path must be absolute");
        }
        endpoint_dir_ = string(xdg_runtime_dir) + "/zmq";
    }

    twoway_timeout_ = get_optional_int(zmq_config_group, twoway_timeout_key, DFLT_ZMQ_TWOWAY_TIMEOUT);
    if (twoway_timeout_ < -1)
    {
        throw_ex("Illegal value (" + to_string(twoway_timeout_) + ") for " + twoway_timeout_key + ": value must be >= -1");
    }

    registry_timeout_ = get_optional_int(zmq_config_group, registry_timeout_key, DFLT_ZMQ_REGISTRY_TIMEOUT);
    if (registry_timeout_ < 10 || registry_timeout_ > 60000)
    {
        throw_ex("Illegal value (" + to_string(registry_timeout_) + ") for " + registry_timeout_key + ": value must be 10-60000");
    }

    locate_timeout_ = get_optional_int(zmq_config_group, locate_timeout_key, DFLT_ZMQ_LOCATE_TIMEOUT);
    if (locate_timeout_ < 10 || locate_timeout_ > 60000)
    {
        throw_ex("Illegal value (" + to_string(locate_timeout_) + ") for " + locate_timeout_key + ": value must be 10-60000");
    }

    child_scopes_timeout_ = get_optional_int(zmq_config_group, child_scopes_timeout_key, DFLT_ZMQ_CHILDSCOPES_TIMEOUT);
    if (child_scopes_timeout_ < 10 || child_scopes_timeout_ > 60000)
    {
        throw_ex("Illegal value (" + to_string(child_scopes_timeout_) + ") for " + child_scopes_timeout_key + ": value must be 10-60000");
    }

    registry_endpoint_dir_ = get_optional_string(zmq_config_group, registry_endpoint_dir_key);
    ss_registry_endpoint_dir_ = get_optional_string(zmq_config_group, ss_registry_endpoint_dir_key);

    KnownEntries const known_entries = {
                                          {  zmq_config_group,
                                             {
                                                endpoint_dir_key,
                                                twoway_timeout_key,
                                                locate_timeout_key,
                                                registry_timeout_key,
                                                child_scopes_timeout_key,
                                                registry_endpoint_dir_key,
                                                ss_registry_endpoint_dir_key
                                             }
                                          }
                                       };
    check_unknown_entries(known_entries);
}

ZmqConfig::~ZmqConfig()
{
}

string ZmqConfig::endpoint_dir() const
{
    return endpoint_dir_;
}

int ZmqConfig::twoway_timeout() const
{
    return twoway_timeout_;
}

int ZmqConfig::locate_timeout() const
{
    return locate_timeout_;
}

int ZmqConfig::registry_timeout() const
{
    return registry_timeout_;
}

int ZmqConfig::child_scopes_timeout() const
{
    return child_scopes_timeout_;
}

string ZmqConfig::registry_endpoint_dir() const
{
    return registry_endpoint_dir_;
}

string ZmqConfig::ss_registry_endpoint_dir() const
{
    return ss_registry_endpoint_dir_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
