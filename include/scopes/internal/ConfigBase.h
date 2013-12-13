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

#ifndef UNITY_API_SCOPES_INTERNAL_CONFIGBASE_H
#define UNITY_API_SCOPES_INTERNAL_CONFIGBASE_H

#include <unity/util/IniParser.h>

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
    ConfigBase(std::string const& configfile);
    virtual ~ConfigBase() noexcept;

    unity::util::IniParser::SPtr parser() const noexcept;

    virtual std::string get_string(std::string const& group, std::string const& key) const;
    virtual std::string get_optional_string(std::string const& group, std::string const& key) const;
    virtual std::string get_middleware(std::string const& group, std::string const& key) const;

    virtual void throw_ex(::std::string const& reason) const;

private:
    unity::util::IniParser::SPtr p() const;
    unity::util::IniParser::SPtr parser_;
    std::string configfile_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
