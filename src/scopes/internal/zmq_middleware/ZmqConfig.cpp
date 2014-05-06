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

#include <unistd.h>
#include <iostream>

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
    const string public_dir_key = "EndpointDir";
}

ZmqConfig::ZmqConfig(string const& configfile) :
    ConfigBase(configfile, DFLT_ZMQ_MIDDLEWARE_INI)
{
    if (!configfile.empty())
    {
        public_dir_ = get_optional_string(zmq_config_group, public_dir_key);
    }

    // Set the endpoint directory if it was not set explicitly.
    // We look for the XDG_RUNTIME_DIR env variable. If that is not
    // set, we give up.
    if (public_dir_.empty())
    {
cerr << "USING XDG" << endl;
        char* xdg_runtime_dir = secure_getenv("XDG_RUNTIME_DIR");
        if (!xdg_runtime_dir || *xdg_runtime_dir == '\0')
        {
            throw ConfigException("No endpoint directories specified, and XDG_RUNTIME_DIR "
                                  "environment variable not set");
        }
        public_dir_ = string(xdg_runtime_dir) + "/zmq";
    }
    private_dir_ = public_dir_ + "/priv";

    const KnownEntries known_entries = {
                                          {  zmq_config_group,
                                             {
                                                public_dir_key
                                             }
                                          }
                                       };
    check_unknown_entries(known_entries);
}

ZmqConfig::~ZmqConfig()
{
}

string ZmqConfig::public_dir() const
{
    return public_dir_;
}

string ZmqConfig::private_dir() const
{
    return private_dir_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
