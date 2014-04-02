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

#include <unity/scopes/internal/ScopeConfig.h>

#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <algorithm>
#include <string>
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
    const string overrideable_str = "Override";
    const string scope_id_str = "DisplayName";
    const string scope_type_str = "Type";
    const string description_str = "Description";
    const string author_str = "Author";
    const string art_str = "Art";
    const string icon_str = "Icon";
    const string search_hint_str = "SearchHint";
    const string hot_key_str = "HotKey";
    const string invisible_str = "Invisible";
    const string scope_runner_exec = "ScopeRunner";

    const string scope_type_trusted_str = "Trusted";
    const string scope_type_untrusted_local_str = "UntrustedLocal";
    const string scope_type_untrusted_internet_str = "UntrustedInternet";
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

    type_ = ScopeType::Trusted;
    try
    {
        string type = parser()->get_string(SCOPE_CONFIG_GROUP, scope_type_str);
        if (type == scope_type_trusted_str)
        {
            type_ = ScopeType::Trusted;
        }
        else if (type == scope_type_untrusted_local_str)
        {
            type_ = ScopeType::UntrustedLocal;
        }
        else if (type == scope_type_untrusted_internet_str)
        {
            type_ = ScopeType::UntrustedInternet;
        }
    }
    catch (LogicException const& e)
    {
    }

    display_name_ = parser()->get_string(SCOPE_CONFIG_GROUP, scope_id_str);
    description_ = parser()->get_string(SCOPE_CONFIG_GROUP, description_str);
    author_ = parser()->get_string(SCOPE_CONFIG_GROUP, author_str);

    // For optional values, we store them in a unique_ptr so we can distinguish the "not set at all" case
    // from the "explicitly set to empty string" case. parser()->get_string throws LogicException if
    // the key is not present, so we ignore the exception for optional values.
    try
    {
        string art = parser()->get_string(SCOPE_CONFIG_GROUP, art_str);
        art_.reset(new string(art));
    }
    catch (LogicException const&)
    {
    }
    try
    {
        string icon = parser()->get_string(SCOPE_CONFIG_GROUP, icon_str);
        icon_.reset(new string(icon));
    }
    catch (LogicException const&)
    {
    }
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
    try
    {
        string key = parser()->get_string(SCOPE_CONFIG_GROUP, invisible_str);
        std::transform(begin(key), end(key), begin(key), ::toupper);
        invisible_.reset(new bool(key == "TRUE"));
    }
    catch (LogicException const&)
    {
    }

    // custom scope runner executable is optional
    try
    {
        string key = parser()->get_string(SCOPE_CONFIG_GROUP, scope_runner_exec);
        scope_runner_.reset(new string(key));
    }
    catch (LogicException const&)
    {
    }

    // read all display attributes from SCOPE_DISPLAY_GROUP config group
    try
    {
        for (auto const& key: parser()->get_keys(SCOPE_DISPLAY_GROUP))
        {
            appearance_attributes_[key] = parser()->get_string(SCOPE_DISPLAY_GROUP, key);
        }
    }
    catch (LogicException const&)
    {
    }
}

ScopeConfig::~ScopeConfig()
{
}

bool ScopeConfig::overrideable() const
{
    return overrideable_;
}

ScopeType ScopeConfig::type() const
{
    return type_;
}

string ScopeConfig::display_name() const
{
    return display_name_;
}

string ScopeConfig::description() const
{
    return description_;
}

string ScopeConfig::author() const
{
    return author_;
}

string ScopeConfig::art() const
{
    if (!art_)
    {
        throw NotFoundException("Art not set", art_str);
    }
    return *art_;
}

string ScopeConfig::icon() const
{
    if (!icon_)
    {
        throw NotFoundException("Icon not set", icon_str);
    }
    return *icon_;
}

string ScopeConfig::search_hint() const
{
    if (!search_hint_)
    {
        throw NotFoundException("Hint not set", search_hint_str);
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

bool ScopeConfig::invisible() const
{
    if (!invisible_)
    {
        return false;
    }
    return *invisible_;
}

string ScopeConfig::scope_runner() const
{
    if (!scope_runner_)
    {
        throw NotFoundException("Runner binary not set", scope_runner_exec);
    }
    return *scope_runner_;
}

VariantMap ScopeConfig::appearance_attributes() const
{
    return appearance_attributes_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
