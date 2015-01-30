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

#include "FindFiles.h"

#include <unity/UnityExceptions.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <map>

using namespace std;
using namespace unity;
using namespace boost;

namespace scoperegistry
{

// Return all paths underneath the given dir that are of the given type
// or are a symbolic link to the given type.

vector<string> find_entries(string const& install_dir, EntryType type)
{
    vector<string> entries;

    if (!filesystem::is_directory(install_dir))
    {
        return entries;
    }

    auto end_it = filesystem::directory_iterator();
    try
    {
        for (filesystem::directory_iterator dir_it(install_dir); dir_it != end_it; ++dir_it)
        {
            if (type == File && !filesystem::is_regular_file(*dir_it))
            {
                continue;
            }
            if (type == Directory && !filesystem::is_directory(*dir_it))
            {
                continue;
            }
            entries.emplace_back(install_dir + "/" + dir_it->path().filename().native());
        }
    }
    catch (std::exception const&)
    {
        // Ignore permission errors and the like.
        // If a scope installation directory lacks permission, we still get
        // get an error later, when we try to establish a watch for the install dir.
    }

    return entries;
}

// Return all files of the form dir/<somescope>.ini (but not <somescope>-setttings.ini)
// that are regular files or symbolic links and have the specified suffix.
// The empty suffix is legal and causes all regular files and symlinks to be returned.

map<string, string> find_scope_dir_configs(string const& scope_dir, string const& suffix)
{
    map<string, string> files;

    auto paths = find_entries(scope_dir, File);
    for (auto path : paths)
    {
        filesystem::path fs_path(path);
        if (fs_path.extension() != suffix)
        {
            continue;
        }
        if (boost::ends_with(path, "-settings.ini"))
        {
            continue;
        }
        auto scope_id = fs_path.stem().native();
        files.insert(make_pair(scope_id, path));
    }

    return files;
}

// Return all files of the form dir/*/<somescope>.ini (but not <somescope>-settings.ini)
// that are regular files or symbolic links and have the specified suffix.
// The empty suffix is legal and causes all regular files and symlinks to be returned.
// Print error message for any scopes with an id that was seen previously.

map<string, string> find_install_dir_configs(string const& install_dir,
                                             string const& suffix,
                                             std::function<void(string const&)> error)
{
    map<string, string> files;
    map<string, string> scopes_seen;

    auto scope_dirs = find_entries(install_dir, Directory);
    for (auto scope_dir : scope_dirs)
    {
        try
        {
            auto configs = find_scope_dir_configs(scope_dir, suffix);
            for (auto config : configs)
            {
                auto const it = scopes_seen.find(config.first);
                if (it != scopes_seen.end())
                {
                    error("ignoring second instance of non-unique scope: " + config.second + "\n"
                            "previous instance: " + it->second);
                    continue;
                }
                scopes_seen[config.first] = config.second;
                files.insert(config);
            }
        }
        catch (FileException const& e)
        {
            error(e.what());
            error("could not open scope directory: " + scope_dir);
        }
    }

    return files;
}

} // namespace scoperegistry
