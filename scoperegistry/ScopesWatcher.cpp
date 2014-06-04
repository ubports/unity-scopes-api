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

ScopesWatcher::ScopesWatcher(RegistryObject::SPtr registry,
                             std::function<void(std::pair<std::string, std::string> const&)> ini_added_callback)
    : registry_(registry)
    , ini_added_callback_(ini_added_callback)
{
}

ScopesWatcher::~ScopesWatcher()
{
    cleanup();
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
                auto config = *configs.cbegin();
                std::lock_guard<std::mutex> lock(mutex_);
                dir_to_ini_map_[subdir] = config.second;
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
        auto config = *configs.cbegin();
        // Associate this directory with the contained config file
        {
            std::lock_guard<std::mutex> lock(mutex_);
            dir_to_ini_map_[dir] = config.second;
        }

        // New config found, execute callback
        ini_added_callback_(config);
        std::cout << "ScopesWatcher: scope: \"" << config.first << "\" installed to: \""
                  << dir << "\"" << std::endl;
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
        std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" uninstalled from: \""
                  << dir << "\"" << std::endl;
    }

    // Remove the watch for this directory
    remove_watch(dir);
}

void ScopesWatcher::watch_event(DirWatcher::EventType event_type,
                                DirWatcher::FileType file_type,
                                std::string const& path)
{
    filesystem::path fs_path(path);

    if (file_type == DirWatcher::File && fs_path.extension() == ".ini")
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string parent_path = fs_path.parent_path().native();
        std::string scope_id = fs_path.stem().native();

        // A .ini has been added / modified
        if (event_type == DirWatcher::Added || event_type == DirWatcher::Modified)
        {
            dir_to_ini_map_[parent_path] = path;
            ini_added_callback_(std::make_pair(scope_id, path));
            std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" installed to: \""
                      << parent_path << "\"" << std::endl;
        }
        // A .ini has been removed
        else if (event_type == DirWatcher::Removed)
        {
            dir_to_ini_map_.erase(parent_path);
            registry_->remove_local_scope(scope_id);
            std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" uninstalled from: \""
                      << parent_path << "\"" << std::endl;
        }
    }
    else if (file_type == DirWatcher::File && fs_path.extension() == ".so")
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string parent_path = fs_path.parent_path().native();

        // Check if this directory is associate with the config file
        if (dir_to_ini_map_.find(parent_path) != dir_to_ini_map_.end())
        {
            std::string ini_path = dir_to_ini_map_.at(parent_path);
            filesystem::path fs_ini_path(ini_path);
            std::string scope_id = fs_ini_path.stem().native();

            // A .so file has been added / modified
            if (event_type == DirWatcher::Added || event_type == DirWatcher::Modified)
            {
                ini_added_callback_(std::make_pair(scope_id, ini_path));
                std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" installed to: \""
                          << parent_path << "\"" << std::endl;
            }
            // A .so file has been removed
            else if (event_type == DirWatcher::Removed)
            {
                registry_->remove_local_scope(scope_id);
                std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" uninstalled from: \""
                          << parent_path << "\"" << std::endl;
            }
        }
    }
    else
    {
        // A new sub directory has been added
        if (event_type == DirWatcher::Added)
        {
            // try add this path as a scope folder
            // (we need to do this with both files and folders added, as the file added may be a symlink)
            try
            {
                add_scope_dir(path);
            }
            catch (...) {}
        }
        // A sub directory has been removed
        else if (event_type == DirWatcher::Removed)
        {
            remove_scope_dir(path);
        }
    }
}

} // namespace scoperegistry
