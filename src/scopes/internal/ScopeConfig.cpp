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

#include <boost/algorithm/string/trim.hpp>

#include <algorithm>
#include <sstream>
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
    const string scope_config_group = "ScopeConfig";
    const string overrideable_key = "Override";
    const string scope_name_key = "DisplayName";
    const string description_key = "Description";
    const string author_key = "Author";
    const string art_key = "Art";
    const string icon_key = "Icon";
    const string search_hint_key = "SearchHint";
    const string hot_key_key = "HotKey";                            // Undocumented, currently unused
    const string invisible_key = "Invisible";                       // Deliberately undocumented
    const string location_data_needed_key = "LocationDataNeeded";
    const string scoperunner_key = "ScopeRunner";
    const string idle_timeout_key = "IdleTimeout";
    const string results_ttl_key = "ResultsTtlType";
    const string debug_mode_key = "DebugMode";                      // Deliberately undocumented
    const string child_scope_ids_key = "ChildScopes";               // Deprecated
    const string version_key = "Version";
    const string keywords_key = "Keywords";
    const string is_aggregator_key = "IsAggregator";

    const string scope_appearance_group = "Appearance";
    const string fg_color_key = "ForegroundColor";
    const string bg_color_key = "BackgroundColor";
    const string shape_images_key = "ShapeImages";
    const string preview_button_color_key = "PreviewButtonColor";
    const string logo_overlay_color_key = "LogoOverlayColor";
    const string pageheader_logo_key = "PageHeader.Logo";
    const string pageheader_fg_color_key = "PageHeader.ForegroundColor";
    const string pageheader_background_key = "PageHeader.Background";
    const string pageheader_div_color_key = "PageHeader.DividerColor";
    const string pageheader_nav_bg_key = "PageHeader.NavigationBackground";
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
    boost::trim(display_name_);
    if (display_name_.empty())
    {
        throw_ex(scope_name_key + " cannot be empty or whitespace only");
    }

    description_ = parser()->get_locale_string(scope_config_group, description_key);
    boost::trim(description_);
    if (description_.empty())
    {
        throw_ex(description_key + " cannot be empty or whitespace only");
    }

    author_ = parser()->get_string(scope_config_group, author_key);
    boost::trim(author_);
    if (author_.empty())
    {
        throw_ex(author_key + " cannot be empty or whitespace only");
    }

    // For optional values, we store them in a unique_ptr so we can distinguish the "not set at all" case
    // from the "explicitly set to empty string" case. parser()->get_string throws LogicException if
    // the key is not present, so we ignore the exception for optional values.
    try
    {
        string art = parser()->get_string(scope_config_group, art_key);
        if (!art.empty() && art[0] == '/')
        {
            art = snap_root() + art;
        }
        art_.reset(new string(art));
    }
    catch (LogicException const&)
    {
    }
    try
    {
        string icon = parser()->get_string(scope_config_group, icon_key);
        if (!icon.empty() && icon[0] == '/')
        {
            icon = snap_root() + icon;
        }
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

    // Values less than 1s and greater than 300s are illegal
    if (idle_timeout_ < 1 || idle_timeout_ > 300)
    {
        throw_ex("Illegal value (" + std::to_string(idle_timeout_) + ") for " + idle_timeout_key +
                 ": value must be >= 1 and <= 300");
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

    try
    {
        child_scope_ids_ = parser()->get_string_array(scope_config_group, child_scope_ids_key);
        cerr << "Warning: the " << child_scope_ids_key << " option is deprecated. Please reimplement ScopeBase::find_child_scopes() instead." << endl;
    }
    catch (LogicException const&)
    {
    }

    for (auto const& id: child_scope_ids_)
    {
        if (id.size() == 0)
        {
            throw_ex("Invalid empty scope id for ChildScopes");
        }
    }

    try
    {
        version_ = get_int(scope_config_group, version_key);
        if (version_ <= 0)
        {
            throw_ex("Version must be > 0");
        }
    }
    catch (LogicException const&)
    {
        version_ = 0;
    }

    try
    {
        auto keywords_vec = parser()->get_string_array(scope_config_group, keywords_key);
        keywords_ = set<string>(keywords_vec.begin(), keywords_vec.end());
    }
    catch (LogicException const&)
    {
    }

    for (auto const& keyword: keywords_)
    {
        if (keyword.empty())
        {
            throw_ex("Invalid empty keyword string found in \"Keywords\" list");
        }
    }

    try
    {
        is_aggregator_ = parser()->get_boolean(scope_config_group, is_aggregator_key);
    }
    catch (LogicException const&)
    {
        is_aggregator_ = false;
    }

    try
    {
        debug_mode_ = parser()->get_boolean(scope_config_group, debug_mode_key);
    }
    catch (LogicException const&)
    {
        debug_mode_ = false;
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
               results_ttl_key,
               debug_mode_key,
               child_scope_ids_key,
               version_key,
               keywords_key,
               is_aggregator_key
           }
        },
        {  scope_appearance_group,
           {
               fg_color_key,
               bg_color_key,
               shape_images_key,
               preview_button_color_key,
               logo_overlay_color_key,
               pageheader_logo_key,
               pageheader_fg_color_key,
               pageheader_background_key,
               pageheader_div_color_key,
               pageheader_nav_bg_key
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
    // TODO: (michi) I'm not happy about this, it's too clever by half. For example, if I set PageHeader.Logo = 42
    // and then run the registry, I get no hint whatsoever that I've done something wrong. Further,
    // when other code later calls get_string() on the Variant for the logo, an exception is thrown.
    // It would be far preferable to explicitly run through the list of known attributes and verify
    // that each attribute parses correctly according to its type. If not, we can produce a meaningful
    // error message:
    //
    // "testscopeA.ini: Attribute ShapeImages in group [Appearance] must have a boolean value"
    //
    // As is, we fall over ages later when the attribute is read and, likely, much of the relevant context
    // is lost:
    //
    // "unity::LogicException: Variant does not contain a string value:
    // " boost::bad_get: failed value get using boost::get'
    //
    // That's what happens, and it's not good enough, because I have no idea which Variant
    // out of the hundreds that are kicking around is to blame, and where it was first set.
    //
    // This affects other attributes here and in other config files too. Anything that isn't of type
    // string needs to be checked whether it parses according to its expected type, with a meaningful
    // message in the exception if not.

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

bool ScopeConfig::debug_mode() const
{
    return debug_mode_;
}

VariantMap ScopeConfig::appearance_attributes() const
{
    return appearance_attributes_;
}

std::vector<std::string> ScopeConfig::child_scope_ids() const
{
    return child_scope_ids_;
}

int ScopeConfig::version() const
{
    return version_;
}

std::set<std::string> ScopeConfig::keywords() const
{
    return keywords_;
}

bool ScopeConfig::is_aggregator() const
{
    return is_aggregator_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
