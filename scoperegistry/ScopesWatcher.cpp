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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include "ScopesWatcher.h"

#include "FindFiles.h"

#include <boost/filesystem/path.hpp>

using namespace unity::scopes::internal;
using namespace boost;

namespace scoperegistry
{

ScopesWatcher::ScopesWatcher(RegistryObject::SPtr const& registry,
                             std::function<void(std::pair<std::string, std::string> const&)> ini_added_callback)
    : registry_(registry)
    , ini_added_callback_(ini_added_callback)
{
}

void ScopesWatcher::add_install_dir(std::string const& dir)
{
    try
    {
        // Add watch for root directory
        add_watch(dir);

        // Add watches for each sub directory in root
        auto subdirs = find_entries(dir, EntryType::Directory);
        for (auto const& subdir : subdirs)
        {
            auto configs = find_scope_dir_configs(subdir, ".ini");
            if (!configs.empty())
            {
                dir_to_ini_map_[subdir] = configs[0];
            }
            add_watch(subdir);
        }
    }
    catch (...) {}
}

void ScopesWatcher::add_scope_dir(std::string const& dir)
{
    auto configs = find_scope_dir_configs(dir, ".ini");
    if (!configs.empty())
    {
        // Associate this directory with the contained config file
        {
            std::lock_guard<std::mutex> lock(mutex_);
            dir_to_ini_map_[dir] = configs[0];
        }

        // New config found, execute callback
        filesystem::path p(configs[0]);
        std::string scope_id = p.stem().native();
        ini_added_callback_(std::make_pair(scope_id, configs[0]));
    }

    // Add a watch for this directory
    add_watch(dir);
}

void ScopesWatcher::remove_scope_dir(std::string const& dir)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if this directory is associate with the config file
    if (dir_to_ini_map_.find(dir) != dir_to_ini_map_.end())
    {
        // Inform the registry that this scope has been removed
        std::string ini_path = dir_to_ini_map_.at(dir);
        dir_to_ini_map_.erase(dir);

        filesystem::path p(ini_path);
        std::string scope_id = p.stem().native();
        registry_->remove_local_scope(scope_id);
    }

    // Remove the watch for this directory
    remove_watch(dir);
}

void ScopesWatcher::watch_event(DirWatcher::EventType event_type,
                                DirWatcher::FileType file_type,
                                std::string const& path)
{
    if (file_type == DirWatcher::Directory)
    {
        // A new sub directory has been added
        if (event_type == DirWatcher::Added)
        {
            add_scope_dir(path);
        }
        // A sub directory has been removed
        else if (event_type == DirWatcher::Removed)
        {
            remove_scope_dir(path);
        }
    }
    else if (file_type == DirWatcher::File && path.substr(path.length() - 4) == ".ini")
    {
        filesystem::path p(path);
        std::string scope_id = p.stem().native();

        // A new config file has been added
        if (event_type == DirWatcher::Added)
        {
            ini_added_callback_(std::make_pair(scope_id, path));
        }
        // A config file has been removed
        else if (event_type == DirWatcher::Removed)
        {
            registry_->remove_local_scope(scope_id);
        }
    }
}

} // namespace scoperegistry
