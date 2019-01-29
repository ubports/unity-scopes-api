/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#pragma once

#include <unity/scopes/Variant.h>
#include <string>
#include <sstream>
#include <sys/stat.h>

#include <boost/filesystem.hpp>

namespace unity
{

namespace scopes
{

namespace internal
{

VariantMap::const_iterator find_or_throw(std::string const& context, VariantMap const& var, std::string const& key);
std::string to_percent_encoding(std::string const& str);
std::string from_percent_encoding(std::string const& str);
std::string uncamelcase(std::string const& str);

template<typename T>
bool convert_to(std::string const& val, Variant& out)
{
    std::stringstream str(val);
    T outval;
    str >> outval;
    if (str)
    {
        out = Variant(outval);
        return true;
    }
    return false;
}

template<>
bool convert_to<bool>(std::string const& val, Variant& out);

int safe_system_call(std::string const& command);

void make_directories(std::string const& path_name, mode_t mode);

std::vector<std::string> split_exec_args(std::string const& id, std::string const& custom_exec);
std::string convert_exec_rel_to_abs(std::string const& id, boost::filesystem::path const& scope_dir, std::string const& custom_exec);

} // namespace internal

} // namespace scopes

} // namespace unity
