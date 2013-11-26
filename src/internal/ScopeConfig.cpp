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

#include <scopes/internal/ScopeConfig.h>

#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace
{
    const string overrideable_str = "Override";
}

ScopeConfig::ScopeConfig(string const& configfile) :
    ConfigBase(configfile)
{
    try
    {
        overrideable_ = parser()->get_boolean(SCOPE_CONFIG_GROUP, overrideable_str);
    }
    catch (LogicException const&)
    {
        overrideable_ = false;
    }
}

ScopeConfig::~ScopeConfig() noexcept
{
}

bool ScopeConfig::overrideable() const
{
    return overrideable_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
