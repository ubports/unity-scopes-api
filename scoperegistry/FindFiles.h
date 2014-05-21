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

#ifndef SCOPEREGISTRY_FINDFILES_H
#define SCOPEREGISTRY_FINDFILES_H

#include <vector>
#include <string>

namespace scoperegistry
{

// Return a vector of all paths found underneath a given dir that are of the given type or are
// a symbolic link.

enum EntryType { File, Directory };
std::vector<std::string> find_entries(std::string const& install_dir, EntryType type);

// Return a vector of file names underneath a scope dir that have the given suffix. Files are
// searched for in the specified directory only, that is, no .ini files in further-nested
// directories will be searched.

std::vector<std::string> find_scope_dir_configs(std::string const& scope_dir, std::string const& suffix);

// Return a vector of file names underneath a scope root install dir that have the given suffix.
// Files are searched for exactly "one level down", that is, if we have a directory structure.
//
// canonical/scopeA/myconfig.ini
// canonical/someScope/someScope.ini
//
// we get those two .ini files, but no .ini files in canonical or underneath
// further-nested directories.

std::vector<std::string> find_install_dir_configs(std::string const& install_dir, std::string const& suffix);

} // namespace scoperegistry

#endif
