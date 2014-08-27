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

#ifndef UNITY_INTERNAL_UTILS_H
#define UNITY_INTERNAL_UTILS_H

#include <unity/scopes/Variant.h>
#include <string>
#include <sstream>

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

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
