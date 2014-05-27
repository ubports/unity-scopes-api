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

    int wd = inotify_add_watch(fd_, path.c_str(), IN_CREATE | IN_MOVED_TO |
                                                  IN_DELETE | IN_MOVED_FROM |
                                                  IN_MODIFY | IN_ATTRIB);
    if (wd < 0)
    {
        throw ResourceException("DirWatcher::add_watch(): failed to add watch for path: \"" +
                                path + "\"");
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
    std::cout << "TEST: remove_watch 1. path = " << path << "\n";
    std::lock_guard<std::mutex> lock(mutex_);

    std::cout << "TEST: remove_watch 2.\n";
    if (thread_state_ == Failed)
    {
        std::cout << "TEST: remove_watch 3.\n";
        std::rethrow_exception(thread_exception_);
    }

    std::cout << "TEST: remove_watch 4.\n";
    for (auto& wd : wds_)
    {
        std::cout << "TEST: remove_watch 5. wd.second = " << wd.second << "\n";
        if (wd.second == path)
        {
            // If this is the last watch, stop the thread
            std::cout << "TEST: remove_watch 6. wds_.size() = " << std::to_string(wds_.size()) << ".\n";
            if (wds_.size() == 1)
            {
                std::cout << "TEST: remove_watch 7.\n";
                thread_state_ = Stopping;
            }

            // Remove watch (causes read to return)
            std::cout << "TEST: remove_watch 8. fd_ = " << std::to_string(fd_) << ", wd.first = " << wd.first << "\n";
            inotify_rm_watch(fd_, wd.first);
            std::cout << "TEST: remove_watch 9.\n";
            wds_.erase(wd.first);
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
            std::cout << "TEST: select(). fd = " << std::to_string(fd_) << "\n";
            int ret = select(fd_ + 1, &fds, nullptr, nullptr, nullptr);
            if (ret < 0)
            {
                throw SyscallException("DirWatcher::watch_thread(): Thread aborted: "
                                       "select() failed on inotify fd (fd = " +
                                       std::to_string(fd_) + ")", errno);
            }
            // Get number of bytes available
            std::cout << "TEST: ioctl()\n";
            ret = ioctl(fd_, FIONREAD, &bytes_avail);
            if (ret < 0)
            {
                throw SyscallException("DirWatcher::watch_thread(): Thread aborted: "
                                       "ioctl() failed on inotify fd (fd = " +
                                       std::to_string(fd_) + ")", errno);
            }
            // Read available bytes
            buffer.resize(bytes_avail);
            std::cout << "TEST: read(). buffer.size() = " << std::to_string(buffer.size()) << "\n";
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
                        std::cout << "TEST: watch_event(). ADD DIR. event_path = " << event_path << "\n";
                        watch_event(Added, Directory, event_path);
                    }
                    else
                    {
                        std::cout << "TEST: watch_event(). ADD FILE. event_path = " << event_path << "\n";
                        watch_event(Added, File, event_path);
                    }
                }
                else if (event->mask & IN_DELETE || event->mask & IN_MOVED_FROM)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        std::cout << "TEST: watch_event(). REM DIR. event_path = " << event_path << "\n";
                        watch_event(Removed, Directory, event_path);
                    }
                    else
                    {
                        std::cout << "TEST: watch_event(). REM FILE. event_path = " << event_path << "\n";
                        watch_event(Removed, File, event_path);
                    }
                }
                else if (event->mask & IN_MODIFY || event->mask & IN_ATTRIB)
                {
                    if (event->mask & IN_ISDIR)
                    {
                        std::cout << "TEST: watch_event(). MOD DIR. event_path = " << event_path << "\n";
                        watch_event(Modified, Directory, event_path);
                    }
                    else
                    {
                        std::cout << "TEST: watch_event(). MOD FILE. event_path = " << event_path << "\n";
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
