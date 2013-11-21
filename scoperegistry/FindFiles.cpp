/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "FindFiles.h"

#include <unity/UnityExceptions.h>
#include <unity/util/ResourcePtr.h>

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

using namespace std;
using namespace unity;

namespace scoperegistry
{

namespace
{

bool has_suffix(string const& s, string const& suffix)
{
    auto s_len = s.length();
    auto suffix_len = suffix.length();
    if (s_len >= suffix_len)
    {
        return s.compare(s_len - suffix_len, suffix_len, suffix) == 0;
    }
    return false;
}

// Return all paths underneath the given dir that are of the given type
// or are a symbolic link.

enum EntryType { File, Directory };

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

} // namespace

// Return all files of the form dir/<somescope>/<scomescope>.ini that are regular files or
// symbolic links and have the specified suffix.
// The empty suffix is legal and causes all regular files and symlinks to be returned.

vector<string> find_scope_config_files(string const& install_dir, string const& suffix)
{
    vector<string> files;

    auto subdirs = find_entries(install_dir, Directory);
    for (auto subdir : subdirs)
    {
        string scope_name = basename(const_cast<char*>(subdir.c_str()));    // basename() modifies its argument
        auto candidates = find_entries(subdir, File);
        for (auto c : candidates)
        {
            string config_name = basename(const_cast<char*>(c.c_str()));    // basename() modifies its argument
            if (config_name == scope_name + suffix)
            {
                files.emplace_back(c);
            }
        }
    }

    return files;
}

// Return all files with the given suffix in dir.

vector<string> find_files(string const& dir, string const& suffix)
{
    vector<string> files;

    auto candidates = find_entries(dir, File);
    for (auto c : candidates)
    {
        if (has_suffix(c, suffix))
        {
            files.emplace_back(c);
        }
    }

    return files;
}

} // namespace scoperegistry
