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

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/internal/Utils.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{
    const string scope_config_group = "ScopeConfig";
    const string overrideable_key = "Override";
    const string scope_name_key = "DisplayName";
    const string description_key = "Description";
    const string author_key = "Author";
    const string art_key = "Art";
    const string icon_key = "Icon";
    const string search_hint_key = "SearchHint";
    const string hot_key_key = "HotKey";
    const string invisible_key = "Invisible";
    const string location_data_needed_key = "LocationDataNeeded";
    const string scoperunner_key = "ScopeRunner";
    const string idle_timeout_key = "IdleTimeout";
    const string results_ttl_key = "ResultsTtlType";

    const string scope_appearance_group = "Appearance";
}

ScopeConfig::ScopeConfig(string const& configfile) :
    ConfigBase(configfile)
{
    try
    {
        overrideable_ = parser()->get_boolean(scope_config_group, overrideable_key);
    }
    catch (LogicException const&)
    {
        overrideable_ = false;
    }

    display_name_ = parser()->get_locale_string(scope_config_group, scope_name_key);
    description_ = parser()->get_locale_string(scope_config_group, description_key);
    author_ = parser()->get_string(scope_config_group, author_key);

    // For optional values, we store them in a unique_ptr so we can distinguish the "not set at all" case
    // from the "explicitly set to empty string" case. parser()->get_string throws LogicException if
    // the key is not present, so we ignore the exception for optional values.
    try
    {
        string art = parser()->get_string(scope_config_group, art_key);
        art_.reset(new string(art));
    }
    catch (LogicException const&)
    {
    }
    try
    {
        string icon = parser()->get_string(scope_config_group, icon_key);
        icon_.reset(new string(icon));
    }
    catch (LogicException const&)
    {
    }
    try
    {
        string hint = parser()->get_locale_string(scope_config_group, search_hint_key);
        search_hint_.reset(new string(hint));
    }
    catch (LogicException const&)
    {
    }
    try
    {
        string key = parser()->get_string(scope_config_group, hot_key_key);
        hot_key_.reset(new string(key));
    }
    catch (LogicException const&)
    {
    }
    try
    {
        invisible_ = parser()->get_boolean(scope_config_group, invisible_key);
    }
    catch (LogicException const&)
    {
        invisible_ = false;
    }
    try
    {
        location_data_needed_ = parser()->get_boolean(scope_config_group, location_data_needed_key);
    }
    catch (LogicException const&)
    {
        location_data_needed_ = false;
    }

    // custom scope runner executable is optional
    try
    {
        string scope_runner = parser()->get_string(scope_config_group, scoperunner_key);
        scope_runner_.reset(new string(scope_runner));
    }
    catch (LogicException const&)
    {
    }

    idle_timeout_ = get_optional_int(scope_config_group, idle_timeout_key, DFLT_SCOPE_IDLE_TIMEOUT);

    // Negative values and values greater than max int (once multiplied by 1000 (s to ms)) are illegal
    const int max_idle_timeout = std::numeric_limits<int>::max() / 1000;
    if (idle_timeout_ < 0 || idle_timeout_ > max_idle_timeout)
    {
        throw_ex("Illegal value (" + std::to_string(idle_timeout_) + ") for " + idle_timeout_key +
                 ": value must be > 0 and <= " + std::to_string(max_idle_timeout));
    }


    results_ttl_type_ = ScopeMetadata::ResultsTtlType::None;
    try
    {
        string orig = parser()->get_string(scope_config_group, results_ttl_key);
        string ttl = orig;
        to_lower(ttl);
        if (ttl.empty() || ttl == "none")
        {
        }
        else if (ttl == "small")
        {
            results_ttl_type_ = ScopeMetadata::ResultsTtlType::Small;
        }
        else if (ttl == "medium")
        {
            results_ttl_type_ = ScopeMetadata::ResultsTtlType::Medium;
        }
        else if (ttl == "large")
        {
            results_ttl_type_ = ScopeMetadata::ResultsTtlType::Large;
        }
        else
        {
            throw_ex("Illegal value (\"" + orig + "\") for " + results_ttl_key);
        }
    }
    catch (LogicException const&)
    {
    }

    // read all display attributes from scope_appearance_group
    try
    {
        for (auto const& key: parser()->get_keys(scope_appearance_group))
        {
            parse_appearance_attribute(appearance_attributes_, key, parser()->get_string(scope_appearance_group, key));
        }
    }
    catch (LogicException const&)
    {
    }

    KnownEntries const known_entries = {
                                          {  scope_config_group,
                                             {
                                                overrideable_key,
                                                scope_name_key,
                                                description_key,
                                                author_key,
                                                art_key,
                                                icon_key,
                                                search_hint_key,
                                                hot_key_key,
                                                invisible_key,
                                                location_data_needed_key,
                                                scoperunner_key,
                                                idle_timeout_key,
                                                results_ttl_key
                                             }
                                          },
                                          // TODO: once we know what appearance attributes are supported,
                                          //       we need to add them here.
                                          {  scope_appearance_group,
                                             {
                                             }
                                          }
                                       };
    check_unknown_entries(known_entries);
}

ScopeConfig::~ScopeConfig()
{
}

void ScopeConfig::parse_appearance_attribute(VariantMap& var, std::string const& key, std::string const& val)
{
    auto i = key.find(".");
    if (i == std::string::npos)
    {
        Variant v;
        if (convert_to<int>(val, v) ||
            convert_to<double>(val, v) ||
            convert_to<bool>(val, v))
        {
            var[uncamelcase(key)] = v;
        }
        else
        {
            var[uncamelcase(key)] = val;
        }
    }
    else
    {
        const std::string keypart = uncamelcase(key.substr(0, i));
        VariantMap vm;
        auto it = var.find(keypart);
        if (it != var.end())
        {
            vm = it->second.get_dict();
        }
        parse_appearance_attribute(vm, key.substr(i+1), val);
        var[uncamelcase(keypart)] = vm;
    }
}

bool ScopeConfig::overrideable() const
{
    return overrideable_;
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
        throw NotFoundException("Art not set", art_key);
    }
    return *art_;
}

string ScopeConfig::icon() const
{
    if (!icon_)
    {
        throw NotFoundException("Icon not set", icon_key);
    }
    return *icon_;
}

string ScopeConfig::search_hint() const
{
    if (!search_hint_)
    {
        throw NotFoundException("Hint not set", search_hint_key);
    }
    return *search_hint_;
}

string ScopeConfig::hot_key() const
{
    if (!hot_key_)
    {
        throw NotFoundException("Hot key not set", hot_key_key);
    }
    return *hot_key_;
}

bool ScopeConfig::invisible() const
{
    return invisible_;
}

bool ScopeConfig::location_data_needed() const
{
    return location_data_needed_;
}

string ScopeConfig::scope_runner() const
{
    if (!scope_runner_)
    {
        throw NotFoundException("Scope runner binary not set", scoperunner_key);
    }
    return *scope_runner_;
}

int ScopeConfig::idle_timeout() const
{
    return idle_timeout_;
}

ScopeMetadata::ResultsTtlType ScopeConfig::results_ttl_type() const
{
    return results_ttl_type_;
}

VariantMap ScopeConfig::appearance_attributes() const
{
    return appearance_attributes_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
