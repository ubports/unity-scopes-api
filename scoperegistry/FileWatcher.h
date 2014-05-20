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

#ifndef SCOPEREGISTRY_FILEWATCHER_H
#define SCOPEREGISTRY_FILEWATCHER_H

#include <condition_variable>
#include <map>
#include <thread>

namespace scoperegistry
{

// FileWatcher watches the directory specified by the "dir" constructor parameter for changes in
// the files and folders contained. If a file or folder is added, removed or modified, a user
// callback is executed (also to be provided on construction).

class FileWatcher final
{
public:
    enum EventType
    {
        Added,
        Removed,
        Modified
    };

    enum FileType
    {
        File,
        Directory
    };

    typedef std::function<void(EventType, FileType, std::string const& file_name)> FileWatcherCallback;

    FileWatcher(FileWatcherCallback callback);
    ~FileWatcher();

    bool add_file_watch(std::string const& path);
    bool remove_file_watch(std::string const& path);

    bool add_dir_watch(std::string const& path);
    bool remove_dir_watch(std::string const& path);

private:
    enum ThreadState
    {
        Running,
        Stopping,
        Failed
    };

    FileWatcherCallback const callback_;

    int fd_;
    std::map<int, std::string> wds_;

    std::thread thread_;
    std::mutex mutex_;
    ThreadState thread_state_;

    void watch_thread();
};

} // namespace scoperegistry

#endif  // SCOPEREGISTRY_FILEWATCHER_H
