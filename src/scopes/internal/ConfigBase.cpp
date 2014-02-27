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

#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <unity/util/IniParser.h>


using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

// If configfile is the empty string, we create a default instance that returns "Zmq" for the middleware
// and throws for the other methods.

ConfigBase::ConfigBase(string const& configfile) :
    parser_(nullptr),
    configfile_(configfile)
{
    if (!configfile.empty())
    {
        parser_ = make_shared<util::IniParser>(configfile.c_str());
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

string ConfigBase::get_optional_string(string const& group, string const& key) const
{
    try
    {
        return parser()->get_string(group, key);
    }
    catch (unity::LogicException const&)
    {
        return string();
    }
}

string ConfigBase::get_middleware(string const& group, string const& key) const
{
    if (!parser_)
    {
        return "Zmq";
    }
    string val = get_string(group, key);
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

} // namespace internal

} // namespace scopes

} // namespace unity
