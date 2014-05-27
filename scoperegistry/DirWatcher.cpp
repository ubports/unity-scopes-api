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

#include <iostream>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace unity;

namespace scoperegistry
{

DirWatcher::DirWatcher()
    : fd_(inotify_init())
    , thread_state_(Running)
    , thread_exception_(nullptr)
{
    // Validate the file descriptor
    if (fd_ < 0)
    {
        throw SyscallException("DirWatcher(): inotify_init() failed on inotify fd (fd = " +
                               std::to_string(fd_) + ")", errno);
    }
}

DirWatcher::~DirWatcher()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (thread_state_ == Failed)
        {
            try
            {
                std::rethrow_exception(thread_exception_);
            }
            catch (std::exception const& e)
            {
                std::cerr << "~DirWatcher(): " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "~DirWatcher(): watch_thread was aborted due to an unknown exception"
                          << std::endl;
            }
        }
        else
        {
            // Set state to Stopping
            thread_state_ = Stopping;

            // Remove watches (causes read to return)
            for (auto& wd : wds_)
            {
                inotify_rm_watch(fd_, wd.first);
            }
            wds_.clear();
        }
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
    std::lock_guard<std::mutex> lock(mutex_);

    if (thread_state_ == Failed)
    {
        std::rethrow_exception(thread_exception_);
    }

    for (auto const& wd : wds_)
    {
        if (wd.second == path)
        {
            throw ResourceException("DirWatcher::add_watch(): failed to add watch for path: \"" +
                                    path + "\". Watch already exists.");
        }
    }

    int wd = inotify_add_watch(fd_, path.c_str(), IN_CREATE | IN_MOVED_TO |
                                                  IN_DELETE | IN_MOVED_FROM |
                                                  IN_MODIFY | IN_ATTRIB);
    if (wd < 0)
    {
        throw ResourceException("DirWatcher::add_watch(): failed to add watch for path: \"" +
                                path + "\". inotify_add_watch() failed. (fd = " +
                                std::to_string(fd_) + ", path = " + path + ")");
    }

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

    if (thread_state_ == Failed)
    {
        std::rethrow_exception(thread_exception_);
    }

    for (auto const& wd : wds_)
    {
        if (wd.second == path)
        {
            // If this is the last watch, stop the thread
            if (wds_.size() == 1)
            {
                thread_state_ = Stopping;
            }

            // Remove watch (causes read to return)
            inotify_rm_watch(fd_, wd.first);
            wds_.erase(wd.first);
            break;
        }
    }
}

void DirWatcher::watch_thread()
{
    try
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd_, &fds);

        int bytes_avail = 0;
        std::string buffer;
        std::string event_path;

        // Poll for notifications until stop is requested
        while (true)
        {
            // Wait for a payload to arrive
            int ret = select(fd_ + 1, &fds, nullptr, nullptr, nullptr);
            if (ret < 0)
            {
                throw SyscallException("DirWatcher::watch_thread(): Thread aborted: "
                                       "select() failed on inotify fd (fd = " +
                                       std::to_string(fd_) + ")", errno);
            }
            // Get number of bytes available
            ret = ioctl(fd_, FIONREAD, &bytes_avail);
            if (ret < 0)
            {
                throw SyscallException("DirWatcher::watch_thread(): Thread aborted: "
                                       "ioctl() failed on inotify fd (fd = " +
                                       std::to_string(fd_) + ")", errno);
            }
            // Read available bytes
            buffer.resize(bytes_avail);
            int bytes_read = read(fd_, &buffer[0], buffer.size());
            if (bytes_read < 0)
            {
                throw SyscallException("DirWatcher::watch_thread(): Thread aborted: "
                                       "read() failed on inotify fd (fd = " +
                                       std::to_string(fd_) + ")", errno);
            }

            // Process event(s) received
            int i = 0;
            while (i < bytes_read)
            {
                struct inotify_event* event = (inotify_event*)&buffer[i];
                {
                    event_path = "";
                    std::lock_guard<std::mutex> lock(mutex_);
                    if (wds_.find(event->wd) != wds_.end())
                    {
                        event_path = wds_.at(event->wd) + "/" + event->name;
                    }
                }

                if (event->mask & IN_CREATE || event->mask & IN_MOVED_TO)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        watch_event(Added, Directory, event_path);
                    }
                    else
                    {
                        watch_event(Added, File, event_path);
                    }
                }
                else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        watch_event(Removed, Directory, event_path);
                    }
                    else
                    {
                        watch_event(Removed, File, event_path);
                    }
                }
                else if (event->mask & IN_MODIFY || event->mask & IN_ATTRIB)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        watch_event(Modified, Directory, event_path);
                    }
                    else
                    {
                        watch_event(Modified, File, event_path);
                    }
                }
                i += sizeof(inotify_event) + event->len;
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
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        std::lock_guard<std::mutex> lock(mutex_);
        thread_state_ = Failed;
        thread_exception_ = std::current_exception();
    }
    catch (...)
    {
        std::cerr << "DirWatcher::watch_thread(): Thread aborted: unknown exception" << std::endl;
        std::lock_guard<std::mutex> lock(mutex_);
        thread_state_ = Failed;
        thread_exception_ = std::current_exception();
    }
}

} // namespace scoperegistry
