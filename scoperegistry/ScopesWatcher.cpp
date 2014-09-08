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

#include <unity/UnityExceptions.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

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

void ScopesWatcher::add_install_dir(std::string const& dir, bool notify)
{
    // Add watch for parent directory
    try
    {
        add_watch(parent_dir(dir));
    }
    catch (unity::ResourceException const&) {} // Ignore already exists exception
    catch (unity::SyscallException const& e )
    {
        std::cerr << "ScopesWatcher::add_install_dir(): parent dir watch: " << e.what() << std::endl;
    }

    try
    {
        // Create a new entry for this install dir into idir_to_sdirs_map_
        if (dir.back() == '/')
        {
            std::lock_guard<std::mutex> lock(mutex_);
            idir_to_sdirs_map_[dir.substr(0, dir.length() - 1)] = std::set<std::string>();
        }
        else
        {
            std::lock_guard<std::mutex> lock(mutex_);
            idir_to_sdirs_map_[dir] = std::set<std::string>();
        }

        // Add watch for root directory
        add_watch(dir);

        // Add watches for each sub directory in root
        auto subdirs = find_entries(dir, EntryType::Directory);
        for (auto const& subdir : subdirs)
        {
            add_scope_dir(subdir, notify);
        }
    }
    catch (unity::ResourceException const& e)
    {
        std::cerr << "ScopesWatcher::add_install_dir(): install dir watch: " << e.what() << std::endl;
    }
    catch (unity::SyscallException const& e)
    {
        std::cerr << "ScopesWatcher::add_install_dir(): install dir watch: " << e.what() << std::endl;
    }
}

std::string ScopesWatcher::parent_dir(std::string const& child_dir)
{
    std::string parent;
    if (child_dir.back() == '/')
    {
        parent = filesystem::path(child_dir.substr(0, child_dir.length() - 1)).parent_path().native();
    }
    else
    {
        parent = filesystem::path(child_dir).parent_path().native();
    }
    return parent;
}

void ScopesWatcher::remove_install_dir(std::string const& dir)
{
    std::set<std::string> scope_dirs;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (idir_to_sdirs_map_.find(dir) != idir_to_sdirs_map_.end())
        {
            scope_dirs = idir_to_sdirs_map_.at(dir);
        }
    }

    for (auto const& scope_dir : scope_dirs)
    {
        remove_scope_dir(scope_dir);
    }

    remove_watch(dir);
}

void ScopesWatcher::add_scope_dir(std::string const& dir, bool notify)
{
    auto configs = find_scope_dir_configs(dir, ".ini");
    if (!configs.empty())
    {
        auto config = *configs.cbegin();
        {
            std::lock_guard<std::mutex> lock(mutex_);

            // Associate this scope with its install directory
            if (idir_to_sdirs_map_.find(parent_dir(dir)) != idir_to_sdirs_map_.end())
            {
                idir_to_sdirs_map_.at(parent_dir(dir)).insert(dir);
            }

            // Associate this directory with the contained config file
            sdir_to_ini_map_[dir] = config.second;
        }

        // New config found, execute callback
        if (notify)
        {
            ini_added_callback_(config);
            std::cout << "ScopesWatcher: scope: \"" << config.first << "\" installed to: \""
                      << dir << "\"" << std::endl;
        }
    }

    // Add a watch for this directory (ignore exception if already exists)
    try
    {
        add_watch(dir);
    }
    catch (unity::ResourceException const&) {}
}

void ScopesWatcher::remove_scope_dir(std::string const& dir)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if this directory is associate with a config file
    if (sdir_to_ini_map_.find(dir) != sdir_to_ini_map_.end())
    {
        // Unassociate this scope with its install directory
        if (idir_to_sdirs_map_.find(parent_dir(dir)) != idir_to_sdirs_map_.end())
        {
            idir_to_sdirs_map_.at(parent_dir(dir)).erase(dir);
        }

        // Unassociate this config file with its scope directory
        std::string ini_path = sdir_to_ini_map_.at(dir);
        sdir_to_ini_map_.erase(dir);

        // Inform the registry that this scope has been removed
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

        if (boost::algorithm::ends_with(path, "-settings.ini"))
        {
            std::string scope_id = fs_path.stem().native();
            boost::algorithm::replace_last(scope_id, "-settings", "");

            // For add/remove/modify of a settings definition, we pretend that the scope's
            // config file was added or modified (provided the .ini file exists). This triggers
            // re-loading the metadata for the scope.
            std::string fs_ini_path = parent_path + "/" + scope_id + ".ini";
            if (boost::filesystem::exists(fs_ini_path))
            {
                ini_added_callback_(std::make_pair(scope_id, fs_ini_path));
                std::string const action = event_type == DirWatcher::Removed ? "uninstalled from" : "installed to";
                std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" settings definition " << action << ": \""
                          << parent_path << "\"" << std::endl;
            }
        }
        else
        {
            std::string scope_id = fs_path.stem().native();

            // A .ini has been added / modified
            if (event_type == DirWatcher::Added || event_type == DirWatcher::Modified)
            {
                sdir_to_ini_map_[parent_path] = path;
                ini_added_callback_(std::make_pair(scope_id, path));
                std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" installed to: \""
                          << parent_path << "\"" << std::endl;
            }
            // A .ini has been removed
            else if (event_type == DirWatcher::Removed)
            {
                sdir_to_ini_map_.erase(parent_path);
                registry_->remove_local_scope(scope_id);
                std::cout << "ScopesWatcher: scope: \"" << scope_id << "\" uninstalled from: \""
                          << parent_path << "\"" << std::endl;
            }
        }
    }
    else if (file_type == DirWatcher::File && fs_path.extension() == ".so")
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string parent_path = fs_path.parent_path().native();

        // Check if this directory is associate with the config file
        if (sdir_to_ini_map_.find(parent_path) != sdir_to_ini_map_.end())
        {
            std::string ini_path = sdir_to_ini_map_.at(parent_path);
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
        bool is_install_dir = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (idir_to_sdirs_map_.find(path) != idir_to_sdirs_map_.end())
            {
                is_install_dir = true;
            }
        }

        // If this path is an install dir:
        if (is_install_dir)
        {
            // An install directory has been added
            if (event_type == DirWatcher::Added)
            {
                add_install_dir(path, true);
            }
            // An install directory has been removed
            else if (event_type == DirWatcher::Removed)
            {
                remove_install_dir(path);
            }
        }
        // Else if this path is within an install dir:
        else if (idir_to_sdirs_map_.find(parent_dir(path)) != idir_to_sdirs_map_.end())
        {
            // A new sub directory has been added
            if (event_type == DirWatcher::Added)
            {
                // try add this path as a scope folder (ignore failures to add this path as scope dir)
                // (we need to do this with both files and folders added, as the file added may be a symlink)
                try
                {
                    add_scope_dir(path, true);
                }
                catch (unity::ResourceException const&) {}
                catch (unity::SyscallException const&) {}
            }
            // A sub directory has been removed
            else if (event_type == DirWatcher::Removed)
            {
                remove_scope_dir(path);
            }
        }
    }
}

} // namespace scoperegistry
