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

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

const char* ZmqConfig::ZMQ_CONFIG_GROUP = "Zmq";

namespace
{
    const string public_dir_str = "EndpointDir.Public";
    const string private_dir_str = "EndpointDir.Private";
}

ZmqConfig::ZmqConfig(string const& configfile) :
    ConfigBase(configfile)
{
    if (configfile.empty())
    {
        public_dir_ = "/tmp";
        private_dir_ = "/tmp";
    }
    else
    {
        public_dir_ = get_string(ZMQ_CONFIG_GROUP, public_dir_str);
        private_dir_ = get_string(ZMQ_CONFIG_GROUP, private_dir_str);
    }
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
