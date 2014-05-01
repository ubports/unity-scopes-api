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

#include <unistd.h>
#include <iostream> // TODO: remove this, debug only

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
    const string public_dir_key = "EndpointDir.Public";
    const string private_dir_key = "EndpointDir.Private";
}

ZmqConfig::ZmqConfig(string const& configfile) :
    ConfigBase(configfile, DFLT_ZMQ_MIDDLEWARE_INI)
{
    if (!configfile.empty())
    {
        public_dir_ = get_optional_string(zmq_config_group, public_dir_key);
        private_dir_ = get_optional_string(zmq_config_group, private_dir_key);
    }

    // Set the endpoint directories if they were not set explicitly.
    // We look for the XDG_RUNTIME_DIR env variable. If that is not
    // set, we default to DFLT_ENDPOINT_DIR_BASE/<effective UID>/zmq.
    if (public_dir_.empty() || private_dir_.empty())
    {
        string basedir;
        char* xdg_runtime_dir = secure_getenv("XDG_RUNTIME_DIR");
cerr << "XDG_RUNTIME_DIR is: " << (xdg_runtime_dir ? (char*)"null" : xdg_runtime_dir) << endl;
system("ls -l /run/user");
system("echo UID: `id`");
        if (!xdg_runtime_dir || *xdg_runtime_dir == '\0')
        {
            basedir = string(DFLT_ENDPOINT_DIR_BASE) + "/" + std::to_string(geteuid()) + "/zmq";
        }
        else
        {
            basedir = string(xdg_runtime_dir) + "/zmq";
        }
        if (public_dir_.empty())
        {
            public_dir_ = basedir;
        }
        if (private_dir_.empty())
        {
            private_dir_ = basedir + "/priv";
        }
    }

    const KnownEntries known_entries = {
                                          {  zmq_config_group,
                                             {
                                                public_dir_key,
                                                private_dir_key
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
