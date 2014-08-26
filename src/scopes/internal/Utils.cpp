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

#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <iomanip>
#include <locale>
#include <mutex>

namespace unity
{

namespace scopes
{

namespace internal
{

VariantMap::const_iterator find_or_throw(std::string const& context, VariantMap const& var, std::string const& key)
{
    auto it = var.find(key);
    if (it == var.end())
    {
        throw unity::scopes::NotFoundException(context + ": missing element", key);
    }
    return it;
}

std::string to_percent_encoding(std::string const& str)
{
    std::ostringstream result;
    for (auto const& c: str)
    {
        if ((!isalnum(c)))
        {
            result << '%' << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(static_cast<unsigned char>(c)) << std::nouppercase;
        }
        else
        {
            result << c;
        }
    }
    return result.str();
}

std::string from_percent_encoding(std::string const& str)
{
    std::ostringstream result;
    for (auto it = str.begin(); it != str.end(); it++)
    {
        auto c = *it;
        if (c == '%')
        {
            bool valid = false;
            // take two characters and covert them from hex to actual char
            if (++it != str.end())
            {
                c = *it;
                if (++it != str.end())
                {
                    std::string const hexnum { c, *it };
                    try
                    {
                        auto k = std::stoi(hexnum, nullptr, 16);
                        result << static_cast<char>(k);
                        valid = true;
                    }
                    catch (std::logic_error const& e) // covers both std::invalid_argument and std::out_of_range
                    {
                        std::stringstream err;
                        err << "from_percent_encoding(): unsupported conversion of '" << hexnum << "'";
                        throw unity::InvalidArgumentException(err.str());
                    }
                }
            }
            if (!valid)
            {
                throw unity::InvalidArgumentException("from_percent_encoding(): too few characters for percent-encoded value");
            }
        }
        else
        {
            result << c;
        }
    }
    return result.str();
}

std::string uncamelcase(std::string const& str)
{
    const std::locale loc("");
    if (str.size() == 0)
    {
        return str;
    }
    auto it = str.begin();
    int previous_is_lower = std::islower(*it);
    std::stringstream res;
    res << std::tolower(*it, loc);
    ++it;
    while (it != str.end())
    {
        if (std::isupper(*it) && previous_is_lower)
        {
            res << "-";
        }
        previous_is_lower = std::islower(*it);
        res << std::tolower(*it, loc);
        ++it;
    }
    return res.str();
}

template<>
bool convert_to<bool>(std::string const& val, Variant& out)
{
    if (val == "true")
    {
        out = Variant(true);
        return true;
    }
    if (val == "false")
    {
        out = Variant(false);
        return true;
    }
    return false;
}

int safe_system_call(std::string const& command)
{
    static std::mutex system_mutex;
    std::lock_guard<std::mutex> lock(system_mutex);
    return std::system(command.c_str());
}

} // namespace internal

} // namespace scopes

} // namespace unity
