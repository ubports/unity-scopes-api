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

#include <scopes/ScopeExceptions.h>
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
    const string scope_name_str = "LocalizedName";
    const string icon_uri_str = "IconURI";
    const string description_str = "Description";
    const string search_hint_str = "SearchHint";
    const string hot_key_str = "HotKey";
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
    icon_uri_ = parser()->get_string(SCOPE_CONFIG_GROUP, icon_uri_str);
    localized_name_ = parser()->get_string(SCOPE_CONFIG_GROUP, scope_name_str);
    description_ = parser()->get_string(SCOPE_CONFIG_GROUP, description_str);

    // For optional values, we store them in a unique_ptr so we can distinguish the "not set at all" case
    // from the "explicitly set to empty string" case.
    try
    {
        string hint = parser()->get_string(SCOPE_CONFIG_GROUP, search_hint_str);
        search_hint_.reset(new string(hint));
    }
    catch (LogicException const&)
    {
    }
    try
    {
        string key = parser()->get_string(SCOPE_CONFIG_GROUP, hot_key_str);
        hot_key_.reset(new string(key));
    }
    catch (LogicException const&)
    {
    }
}

ScopeConfig::~ScopeConfig() noexcept
{
}

bool ScopeConfig::overrideable() const
{
    return overrideable_;
}

string ScopeConfig::icon_uri() const
{
    return icon_uri_;
}

string ScopeConfig::localized_name() const
{
    return localized_name_;
}

string ScopeConfig::description() const
{
    return description_;
}

string ScopeConfig::search_hint() const
{
    if (!search_hint_)
    {
        throw NotFoundException("Key not set", search_hint_str);
    }
    return *search_hint_;
}

string ScopeConfig::hot_key() const
{
    if (!hot_key_)
    {
        throw NotFoundException("Key not set", hot_key_str);
    }
    return *hot_key_;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
