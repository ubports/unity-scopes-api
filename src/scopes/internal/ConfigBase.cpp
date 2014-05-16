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

#include <unity/scopes/internal/ConfigBase.h>

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <unity/util/IniParser.h>

#include <boost/filesystem/path.hpp>

#include <iostream>
#include <locale>
#include <sys/stat.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

// If configfile is the empty string, we create a default instance that returns "Zmq" for the middleware
// and throws for the other methods.

ConfigBase::ConfigBase(string const& configfile, string const& dflt_file) :
    parser_(nullptr),
    configfile_(configfile)
{
    if (!configfile.empty())
    {
        boost::filesystem::path path(configfile);
        if (path.extension() != ".ini")
        {
            throw ConfigException(string("invalid config file name: \"") + configfile + "\": missing .ini extension");
        }
    }
    if (configfile.empty() || configfile == dflt_file)
    {
        // No configfile was specified or it was the same as dflt_file.
        // We check whether dflt_file exists. If so, we use it. Otherwise,
        // we don't have a configfile at all and run with defaults.
        if (path_exists(dflt_file))
        {
            parser_ = make_shared<util::IniParser>(dflt_file.c_str());
            configfile_ = dflt_file;
        }
    }
    else
    {
        // The configfile was specified and differs from dflt_file,
        // so we use it.
        if (configfile != dflt_file)
        {
            parser_ = make_shared<util::IniParser>(configfile.c_str());
            configfile_ = configfile;
        }
    }
}

ConfigBase::~ConfigBase()
{
}

util::IniParser::SPtr ConfigBase::parser() const
{
    if (!parser_)
    {
        throw LogicException("ConfigBase: no parser available with default config");
    }
    return parser_;
}

string ConfigBase::get_string(string const& group, string const& key) const
{
    string val = parser()->get_string(group, key);
    if (val.empty())
    {
        throw_ex("Illegal empty value for " + key);
    }
    return val;
}

string ConfigBase::get_optional_string(string const& group, string const& key, string const& dflt) const
{
    try
    {
        if (!parser_)
        {
            return dflt;
        }
        return parser()->get_string(group, key);
    }
    catch (unity::LogicException const&)
    {
        return dflt;
    }
}

int ConfigBase::get_int(string const& group, string const& key) const
{
    return parser()->get_int(group, key);
}

int ConfigBase::get_optional_int(string const& group, string const& key, int dflt) const
{
    try
    {
        if (!parser_)
        {
            return dflt;
        }
        return parser()->get_int(group, key);
    }
    catch (unity::LogicException const&)
    {
        return dflt;
    }
}

string ConfigBase::get_middleware(string const& group, string const& key) const
{
    if (!parser_)
    {
        return DFLT_MIDDLEWARE;
    }
    string val = get_optional_string(group, key);
    if (val.empty())
    {
        return DFLT_MIDDLEWARE;
    }
    if (val != "Zmq" && val != "REST")
    {
        throw_ex("Illegal value for " + key + ": \"" + val +
                 "\": legal values are \"Zmq\" and \"REST\"");
    }
    return val;
}

void ConfigBase::throw_ex(string const& reason) const
{
    string s = "\"" + configfile_ + "\": " + reason;
    throw ConfigException(s);
}

bool ConfigBase::path_exists(string const& path) const
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

// Check whether a configuration file contains unknown groups
// or unknown keys within a known group. This is useful to catch typos.
// For example, if a config file contains "Smartscopes.Registry.Idenity" instead
// of "Smartscopes.Registry.Identity" (did you spot it?), we print a warning,
// so people don't endlessly scratch their heads as to why the config
// isn't working for them.
//
// KnownEntries is a map of <group, set<key>> pairs that contains
// the known keys for each group. Anything in the config file that
// is not found in the map generates a warning.

void ConfigBase::check_unknown_entries(KnownEntries const& known_entries) const
{
    if (!parser_)
    {
        return;
    }
    auto const groups = parser()->get_groups();
    for (auto const& group : groups)
    {
        auto const it = known_entries.find(group);
        if (it == known_entries.end())
        {
            cerr << "warning: ignoring unknown group " << group << " in file " << configfile_ << endl;
            continue;
        }
        auto keys = parser()->get_keys(group);
        for (auto const& key : keys)
        {
            if (it->second.find(key) == it->second.end())
            {
                // Key not found as presented.
                // Check whether the key is present as a localized entry of
                // the form key[locale_name] = value.
                auto start_pos = key.find('[');
                bool is_localized = start_pos != string::npos &&                    // key contains [
                                    key[key.size() - 1] == ']';                     // key ends with ]
                if (!is_localized ||
                    it->second.find(key.substr(0, start_pos)) == it->second.end())  // match prefix up to [
                {
                    cerr << "warning: ignoring unknown key " << key << " in group " << group
                         << " in file " << configfile_ << endl;
                }
            }
        }
    }
}

void ConfigBase::to_lower(string & str)
{
    locale locale("");
    const ctype<char>& ct = use_facet<ctype<char> >(locale);
    transform(str.begin(), str.end(), str.begin(),
            bind1st(std::mem_fun(&ctype<char>::tolower), &ct));
}

} // namespace internal

} // namespace scopes

} // namespace unity
