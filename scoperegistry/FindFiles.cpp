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
#include <unity/util/ResourcePtr.h>

#include <boost/filesystem/path.hpp>

#include <map>

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;
using namespace unity;
using namespace boost;

namespace scoperegistry
{

// Return all paths underneath the given dir that are of the given type
// or are a symbolic link.

vector<string> find_entries(string const& install_dir, EntryType type)
{
    DIR* d = opendir(install_dir.c_str());
    if (d == NULL)
    {
        throw ResourceException("cannot open scope installation directory \"" + install_dir + "\": " + strerror(errno));
    }
    util::ResourcePtr<DIR*, decltype(&closedir)> dir_ptr(d, closedir);  // Clean up automatically

    vector<string> entries;

    struct dirent* entry;
    while ((entry = readdir(dir_ptr.get())) != NULL)
    {
        string name = entry->d_name;
        if (name == "." || name == "..")
        {
            continue;   // Ignore current and parent dir
        }
        struct stat st;
        int rc = lstat((install_dir + "/" + name).c_str(), &st);
        if (rc == -1)
        {
            continue;   // We ignore errors, such as a file having been unlinked in the mean time.
        }
        if (type == File && !S_ISREG(st.st_mode) && !S_ISLNK(st.st_mode))
        {
            continue;   // Not a regular file or symbolic link
        }
        else if (type == Directory && !S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode))
        {
            continue;   // Not a directory or symbolic link
        }
        entries.emplace_back(install_dir + "/" + name);
    }

    return entries;
}

// Return all files of the form dir/<scomescope>.ini that are regular files or
// symbolic links and have the specified suffix.
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
        auto scope_id = fs_path.stem().native();
        files.insert(make_pair(scope_id, path));
    }

    return files;
}

// Return all files of the form dir/*/<scomescope>.ini that are regular files or
// symbolic links and have the specified suffix.
// The empty suffix is legal and causes all regular files and symlinks to be returned.
// Print error message for any scopes with an id that was seen previously.

map<string, string> find_install_dir_configs(string const& install_dir,
                                             string const& suffix,
                                             function<void(string const&)> error)
{
    map<string, string> files;
    map<string, string> scopes_seen;

    auto scope_dirs = find_entries(install_dir, Directory);
    for (auto scope_dir : scope_dirs)
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

    return files;
}

} // namespace scoperegistry
