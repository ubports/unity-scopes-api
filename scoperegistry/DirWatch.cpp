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

#include "DirWatch.h"

#include <unity/UnityExceptions.h>

#include <sys/inotify.h>
#include <unistd.h>

static const int c_event_size = sizeof(struct inotify_event);
static const int c_buffer_len = 1024 * (c_event_size + 16);

using namespace unity;

namespace scoperegistry
{

DirWatch::DirWatch(DirWatchCallback callback)
    : callback_(callback)
    , thread_state_(Running)
{
    // Create the file descriptor
    fd_ = inotify_init();
    if (fd_ < 0)
    {
        throw ResourceException("DirWatch(): inotify_init() failed");
    }

    // Start the watch thread
    thread_ = std::thread(&DirWatch::watch_thread, this);
}

DirWatch::~DirWatch()
{
    // Set state to Stopping
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_state_ = Stopping;
    }

    // Remove file descriptors from watches (causing read to return)
    for (auto& wd : wds_)
    {
        inotify_rm_watch(fd_, wd.first);
    }
    wds_.clear();

    if (thread_.joinable())
    {
        thread_.join();
    }

    // Close the file descriptor
    close(fd_);
}

bool DirWatch::add_file_watch(std::string const& path)
{
    int wd = inotify_add_watch(fd_, path.c_str(), IN_CREATE | IN_MOVE_SELF | IN_MOVED_TO |
                                                  IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM |
                                                  IN_MODIFY | IN_ATTRIB);
    if (wd < 0)
    {
        return false;
    }

    wds_[wd] = path;
    return true;
}

bool DirWatch::remove_file_watch(std::string const& path)
{
    bool found_path = false;
    for (auto& wd : wds_)
    {
        if (wd.second == path)
        {
            inotify_rm_watch(fd_, wd.first);
            wds_.erase(wd.first);
            found_path = true;
        }
    }
    return found_path;
}

bool DirWatch::add_dir_watch(std::string const& path)
{
    std::string dir_path = path;
    if (dir_path.back() != '/')
    {
        dir_path += '/';
    }

    return add_file_watch(dir_path);
}

bool DirWatch::remove_dir_watch(std::string const& path)
{
    std::string dir_path = path;
    if (dir_path.back() != '/')
    {
        dir_path += '/';
    }

    return remove_file_watch(dir_path);
}

void DirWatch::watch_thread()
{
    try
    {
        // Poll for notifications until stop
        char buffer[c_buffer_len];
        while (true)
        {
            // Wait for changes to directory
            int length = read(fd_, buffer, c_buffer_len);
            if (length < 0)
            {
                throw ResourceException("DirWatch::watch_thread(): failed to read from inotify fd");
            }

            int i = 0;
            while (i < length)
            {
                struct inotify_event* event = (inotify_event*)&buffer[i];
                if (event->mask & IN_MOVE_SELF || event->mask & IN_DELETE_SELF)
                {
                    if (wds_.find(event->wd) != wds_.end())
                    {
                        if (wds_.at(event->wd).back() == '/')
                        {
                            std::string path = wds_.at(event->wd);
                            remove_dir_watch(path);
                            path.resize(path.length() - 1);
                            callback_(Removed, Directory, path);
                        }
                        else
                        {
                            remove_file_watch(wds_.at(event->wd));
                            callback_(Removed, File, wds_.at(event->wd));
                        }
                    }
                }
                else if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        add_dir_watch(wds_.at(event->wd) + event->name);
                        callback_(Added, Directory, wds_.at(event->wd) + event->name);
                    }
                    else
                    {
                        callback_(Added, File, wds_.at(event->wd) + event->name);
                    }
                }
                else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        remove_dir_watch(wds_.at(event->wd) + event->name);
                        callback_(Removed, Directory, wds_.at(event->wd) + event->name);
                    }
                    else
                    {
                        callback_(Removed, File, wds_.at(event->wd) + event->name);
                    }
                }
                else if (event->mask & IN_MODIFY || event->mask & IN_ATTRIB)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        callback_(Modified, Directory, wds_.at(event->wd) + event->name);
                    }
                    else
                    {
                        callback_(Modified, File, wds_.at(event->wd) + event->name);
                    }
                }
                i += c_event_size + event->len;
            }

            // Break from the loop if we are stopping
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (thread_state_ == Stopping)
                {
                    break;
                }
            }
        }
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        thread_state_ = Failed;
    }
}

} // namespace scoperegistry
