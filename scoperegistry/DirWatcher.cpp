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

#include "DirWatcher.h"

#include <unity/UnityExceptions.h>

#include <sys/inotify.h>
#include <unistd.h>

static const int c_event_size = sizeof(struct inotify_event);
static const int c_buffer_len = 1024 * (c_event_size + 16);

using namespace unity;

namespace scoperegistry
{

DirWatcher::DirWatcher(DirWatcherCallback callback)
    : fd_(inotify_init())
    , callback_(callback)
    , thread_state_(Running)
{
    // Validate the file descriptor
    if (fd_ < 0)
    {
        throw ResourceException("DirWatcher(): inotify_init() failed");
    }
}

DirWatcher::~DirWatcher()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Set state to Stopping
        thread_state_ = Stopping;

        // Remove file descriptors from watches (causes read to return)
        for (auto& wd : wds_)
        {
            inotify_rm_watch(fd_, wd.first);
        }
        wds_.clear();
    }

    // Wait for thread to terminate
    if (thread_.joinable())
    {
        thread_.join();
    }

    // Close the file descriptor
    close(fd_);
}

void DirWatcher::add_watch(std::string const& path)
{
    int wd = inotify_add_watch(fd_, path.c_str(), IN_CREATE | IN_MOVED_TO |
                                                  IN_DELETE | IN_MOVED_FROM |
                                                  IN_MODIFY | IN_ATTRIB);
    if (wd < 0)
    {
        throw ResourceException("DirWatcher::add_watch(): failed to add watch for path: \"" + path + "\"");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    wds_[wd] = path;

    // If this is the first watch, start the thread
    if (wds_.size() == 1)
    {
        thread_ = std::thread(&DirWatcher::watch_thread, this);
    }
}

void DirWatcher::remove_watch(std::string const& path)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& wd : wds_)
    {
        if (wd.second == path)
        {
            // If this is the last watch, stop the thread
            if (wds_.size() == 1)
            {
                thread_state_ = Stopping;
            }

            // Remove file descriptor from watch
            inotify_rm_watch(fd_, wd.first);
            wds_.erase(wd.first);
        }
    }
}

void DirWatcher::watch_thread()
{
    try
    {
        // Poll for notifications until stop is requested
        char buffer[c_buffer_len];
        std::string event_path;
        while (true)
        {
            // Wait for changes to directories
            int length = read(fd_, buffer, c_buffer_len);
            if (length < 0)
            {
                throw ResourceException("DirWatcher::watch_thread(): failed to read from inotify fd");
            }

            int i = 0;
            while (i < length)
            {
                struct inotify_event* event = (inotify_event*)&buffer[i];
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    event_path = wds_.at(event->wd) + "/" + event->name;
                }

                if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        callback_(Added, Directory, event_path);
                    }
                    else
                    {
                        callback_(Added, File, event_path);
                    }
                }
                else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        callback_(Removed, Directory, event_path);
                    }
                    else
                    {
                        callback_(Removed, File, event_path);
                    }
                }
                else if (event->mask & IN_MODIFY || event->mask & IN_ATTRIB)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        callback_(Modified, Directory, event_path);
                    }
                    else
                    {
                        callback_(Modified, File, event_path);
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
