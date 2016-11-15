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
#include <unity/util/ResourcePtr.h>

#include <boost/filesystem.hpp>

#include <iomanip>
#include <locale>
#include <mutex>
#include <wordexp.h>

#include <sys/stat.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

VariantMap::const_iterator find_or_throw(string const& context, VariantMap const& var, string const& key)
{
    auto it = var.find(key);
    if (it == var.end())
    {
        throw unity::scopes::NotFoundException(context + ": missing element", key);
    }
    return it;
}

string to_percent_encoding(string const& str)
{
    ostringstream result;
    for (auto const& c: str)
    {
        if ((!isalnum(c)))
        {
            result << '%' << setw(2) << setfill('0') << hex << uppercase << static_cast<int>(static_cast<unsigned char>(c)) << nouppercase;
        }
        else
        {
            result << c;
        }
    }
    return result.str();
}

string from_percent_encoding(string const& str)
{
    ostringstream result;
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
                    string const hexnum { c, *it };
                    try
                    {
                        auto k = stoi(hexnum, nullptr, 16);
                        result << static_cast<char>(k);
                        valid = true;
                    }
                    catch (logic_error const&) // covers both invalid_argument and out_of_range
                    {
                        stringstream err;
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

string uncamelcase(string const& str)
{
    const locale loc("C"); // Use "C" to avoid the Turkish I problem
    if (str.size() == 0)
    {
        return str;
    }
    auto it = str.begin();
    int previous_is_lower = islower(*it);
    stringstream res;
    res << tolower(*it, loc);
    ++it;
    while (it != str.end())
    {
        if (isupper(*it) && previous_is_lower)
        {
            res << "-";
        }
        previous_is_lower = islower(*it);
        res << tolower(*it, loc);
        ++it;
    }
    return res.str();
}

template<>
bool convert_to<bool>(string const& val, Variant& out)
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

int safe_system_call(string const& command)
{
    static mutex system_mutex;
    lock_guard<mutex> lock(system_mutex);
    return system(command.c_str());
}

// Recursively create the directories in path, setting permissions to the specified mode
// (regardless of the setting of umask). If one or more directories already exist, they
// are left unchanged (including their permissions). If a directory cannot be created,
// fail silently.

void make_directories(string const& path_name, mode_t mode)
{
    using namespace boost::filesystem;

    // We can't use boost::create_directories() here because that does not allow control over permissions.
    auto abs = absolute(path_name);
    path path_so_far = "";
    path::iterator it = abs.begin();
    ++it; // No point in trying to create /
    while (it != abs.end())
    {
        path_so_far += "/";
        path_so_far += *it++;
        string p = path_so_far.native();
        if (mkdir(p.c_str(), mode) != 0)
        {
            if (errno == EEXIST)
            {
                continue;
            }
            return;  // No point in continuing, we'd fail on all subsequent iterations.
        }
        // We just created the dir, make sure it has the requested permissions,
        // not the permissions modified by umask.
        chmod(p.c_str(), mode);
    }
}

vector<string> split_exec_args(string const& id, string const& custom_exec)
{
    if (custom_exec.empty())
    {
        throw unity::InvalidArgumentException("Invalid empty executable for scope: '" + id + "'");
    }

    wordexp_t exp;
    vector<string> result;

    // Split command into program and args
    if (wordexp(custom_exec.c_str(), &exp, WRDE_NOCMD) == 0)
    {
        util::ResourcePtr<wordexp_t*, decltype(&wordfree)> free_guard(&exp, wordfree);
        for (size_t i = 0; i < exp.we_wordc; ++i )
        {
            auto argument = string(exp.we_wordv[i]);
            if(argument.find_first_of(' ') != std::string::npos)
            {
                // This argument contains spaces, enclose it in quotation marks
                result.emplace_back("\"" + argument + "\"");
            }
            else
            {
                result.emplace_back(argument);
            }
        }
    }
    else
    {
        throw unity::InvalidArgumentException("Invalid executable for scope: '" + id + "'");
    }

    return result;
}

string convert_exec_rel_to_abs(string const& id, boost::filesystem::path const& scope_dir, string const& custom_exec)
{
    string result;

    // Loop through each argument of the scope runner command and ensure all path args are absolute
    auto custom_exec_args = split_exec_args(id, custom_exec);
    for (auto custom_exec_arg : custom_exec_args)
    {
        boost::filesystem::path argument(custom_exec_arg);
        if (argument.is_relative())
        {
            // First look inside the arch-specific directory
            if (boost::filesystem::exists(scope_dir / DEB_HOST_MULTIARCH / argument))
            {
                // Append the argument as a relative path
                result += (scope_dir / DEB_HOST_MULTIARCH / argument).native() + " ";
            }
            // Next try in the non arch-aware directory
            else if (boost::filesystem::exists(scope_dir / argument))
            {
                // Append the argument as a relative path
                result += (scope_dir / argument).native() + " ";
            }
            // If this is the first argument (program name) it must exist, throw here
            else if (result.empty())
            {
                throw unity::InvalidArgumentException(
                        "Nonexistent scope runner executable: '" + custom_exec_arg
                                + "' for scope: '" + id + "'");
            }
            // Otherwise just append the argument as is
            else
            {
                result += custom_exec_arg + " ";
            }
        }
        else
        {
            // The path provided is already absolute. Ensure that the SNAP env var is honored.

            char const* sroot = getenv("SNAP");
            std::string snap_root;
            if (sroot)
            {
                snap_root = sroot;
                if (!snap_root.empty() && snap_root[snap_root.size() - 1] != '/')
                {
                    snap_root += '/';
                }
            }

            result += snap_root + custom_exec_arg + " ";
        }
    }
    result.resize(result.size() - 1);
    return result;
}

} // namespace internal

} // namespace scopes

} // namespace unity
