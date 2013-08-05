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

#include <unity/api/scopes/internal/RuntimeConfig.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

const char* RuntimeConfig::RUNTIME_CONFIG_GROUP = "Runtime";

RuntimeConfig::RuntimeConfig(string const& configfile) :
    ConfigBase(configfile)
{
    registry_identity_ = parser()->get_string(RUNTIME_CONFIG_GROUP, "Registry.Identity");
    if (registry_identity_.empty())
    {
        throw_ex("Illegal empty value for Registry.Identity");
    }

    registry_configfile_ = parser()->get_string(RUNTIME_CONFIG_GROUP, "Registry.Configfile");
    if (registry_identity_.empty())
    {
        throw_ex("Illegal empty value for Registry.Configfile");
    }

    default_middleware_ = parser()->get_string(RUNTIME_CONFIG_GROUP, "Default.Middleware");
    if (default_middleware_.empty())
    {
        throw_ex("Illegal empty value for Default.ClientMiddleware");
    }

    factory_configfile_ = parser()->get_string(RUNTIME_CONFIG_GROUP, "Factory.ConfigFile");
    if (factory_configfile_.empty())
    {
        throw_ex("Illegal empty value for Factory.ConfigFile");
    }
}

RuntimeConfig::~RuntimeConfig() noexcept
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

string RuntimeConfig::factory_configfile() const
{
    return factory_configfile_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
