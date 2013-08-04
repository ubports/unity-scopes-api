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

#include <unity/api/scopes/internal/RegistryConfig.h>

#include <unity/api/scopes/ScopeExceptions.h>

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
        throw ConfigException("Registry identity cannot be an empty string");
    }
    mw_kind_ = parser()->get_string(REGISTRY_CONFIG_GROUP, "Middleware");
    endpoint_ = parser()->get_string(REGISTRY_CONFIG_GROUP, mw_kind_ + ".Endpoint");
    mw_configfile_ = parser()->get_string(REGISTRY_CONFIG_GROUP, mw_kind_ + ".Configfile");
}

RegistryConfig::~RegistryConfig() noexcept
{
}

string RegistryConfig::mw_kind() const
{
    return mw_kind_;
}

string RegistryConfig::identity() const
{
    return identity_;
}

string RegistryConfig::endpoint() const
{
    return endpoint_;
}

string RegistryConfig::mw_configfile() const
{
    return mw_configfile_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
