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

#pragma once

#include <unity/util/IniParser.h>

#include <map>
#include <set>

namespace unity
{

namespace util
{
class IniParser;
}

namespace scopes
{

namespace internal
{

class ConfigBase
{
public:
    ConfigBase();
    ConfigBase(std::string const& configfile, std::string const& dflt_file = "");
    virtual ~ConfigBase();

    unity::util::IniParser::SPtr parser() const;

    virtual std::string get_string(std::string const& group, std::string const& key) const;
    virtual std::string get_optional_string(std::string const& group,
                                            std::string const& key,
                                            std::string const& dflt = "") const;

    virtual int get_int(std::string const& group, std::string const& key) const;
    virtual int get_optional_int(std::string const& group,
                                 std::string const& key,
                                 int dflt) const;

    virtual std::string get_middleware(std::string const& group, std::string const& key) const;

protected:
    std::string snap_root() const;
    void throw_ex(::std::string const& reason) const;
    bool path_exists(::std::string const& path) const;

    typedef std::map<std::string, std::set<std::string>> KnownEntries;
    void check_unknown_entries(KnownEntries const& valid) const;

    static void to_lower(std::string & str);

private:
    unity::util::IniParser::SPtr parser_;
    std::string configfile_;
    std::string snap_root_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
