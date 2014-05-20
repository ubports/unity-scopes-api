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

DirWatch::DirWatch(std::string const& dir, DirWatchCallback callback)
    : dir_(dir)
    , callback_(callback)
    , thread_state_(Running)
{
    // Create the watch
    fd_ = inotify_init();
    if (fd_ < 0)
    {
        throw ResourceException("DirWatch(): inotify_init() failed");
    }
    wd_ = inotify_add_watch(fd_, dir_.c_str(), IN_CREATE | IN_DELETE | IN_MOVE);

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

    // Remove file descriptor from watch (causing read to return)
    inotify_rm_watch(fd_, wd_);

    if (thread_.joinable())
    {
        thread_.join();
    }

    // Close the file descriptor
    close(fd_);
}

void DirWatch::watch_thread()
{
    try
    {
        // Poll for notifications until stop
        char buffer[c_buffer_len];
        while (true)
        {
            // Poll for changes to directory
            int length = read(fd_, buffer, c_buffer_len);
            if (length < 0)
            {
                throw ResourceException("DirWatch::watch_thread(): failed to read from inotify fd");
            }

            int i = 0;
            while (i < length)
            {
                struct inotify_event* event = (inotify_event*)&buffer[i];
                if (event->len)
                {
                    if (event->mask & IN_CREATE)
                    {
                        if (event->mask & IN_ISDIR)
                        {
                            //printf("The directory %s was created.\n", event->name);
                        }
                        else
                        {
                            //printf("The file %s was created.\n", event->name);
                        }
                    }
                    else if (event->mask & IN_DELETE)
                    {
                        if (event->mask & IN_ISDIR)
                        {
                            //printf("The directory %s was deleted.\n", event->name);
                        }
                        else
                        {
                            //printf("The file %s was deleted.\n", event->name);
                        }
                    }
                    else if (event->mask & IN_MOVE)
                    {
                        if (event->mask & IN_ISDIR)
                        {
                            //printf("The directory %s was moved.\n", event->name);
                        }
                        else
                        {
                            //printf("The file %s was moved.\n", event->name);
                        }
                    }
                    else if (event->mask & IN_MODIFY)
                    {
                        if (event->mask & IN_ISDIR)
                        {
                            //printf("The directory %s was modified.\n", event->name);
                        }
                        else
                        {
                            //printf("The file %s was modified.\n", event->name);
                        }
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
