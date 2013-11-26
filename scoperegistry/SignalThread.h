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

#ifndef SCOPEREGISTRY_SIGNAL_THREAD_H
#define SCOPEREGISTRY_SIGNAL_THREAD_H

#include <unity/util/NonCopyable.h>

#include <unordered_map>
#include <mutex>
#include <thread>

#include <sys/types.h>

namespace scoperegistry
{

class SignalThread final : private unity::util::NonCopyable
{
public:
    SignalThread();
    ~SignalThread() noexcept;

    void add_child(pid_t pid, char const* argv[]);
    void reset_sigs();

private:
    void wait_for_sigs();

    std::unordered_map<pid_t, std::string> children_;
    std::mutex mutex_;
    bool done_;
    std::thread handler_thread_;
    sigset_t sigs_;
};

} // namespace scoperegistry

#endif
