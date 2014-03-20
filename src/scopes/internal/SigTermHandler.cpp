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

#include <unity/scopes/internal/SigTermHandler.h>

#include <unity/UnityExceptions.h>

#include <cassert>
#include <cstring>
#include <iostream>

#include <signal.h>
#include <unistd.h>

#include <poll.h>

#include <sys/eventfd.h>
#include <sys/signalfd.h>

using namespace std;
using namespace unity;

namespace unity
{

namespace scopes
{

namespace internal
{

SigTermHandler::SigTermHandler() :
    event_fd_(-1),
    done_(false)
{
    // Block signals in the caller, so they are delivered to the thread we are about to create.
    sigemptyset(&sigs_);
    sigaddset(&sigs_, SIGINT);
    sigaddset(&sigs_, SIGHUP);
    sigaddset(&sigs_, SIGTERM);

    event_fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);

    if (event_fd_ == -1)
    {
        throw SyscallException("eventfd creation failed", errno);
    }

    // Run a signal handling thread that waits for any of the above signals.
    lock_guard<mutex> lock(mutex_);
    handler_thread_ = thread([this]{ this->wait_for_sigs(); });
}

SigTermHandler::~SigTermHandler()
{
    stop();
    if (handler_thread_.joinable())
        handler_thread_.join();

    ::close(event_fd_);
}

void SigTermHandler::set_callback(function<void()> callback)
{
    lock_guard<mutex> lock(mutex_);
    callback_ = callback;
}

void SigTermHandler::stop()
{
    lock_guard<mutex> lock(mutex_);
    if (!done_)
    {
        done_ = true;
        static const std::int64_t value = {1};
        if (sizeof(value) != ::write(event_fd_, &value, sizeof(value)))
            throw SyscallException("Signalling eventfd failed", errno);
    }
}

// Wait for termination signals. When a termination signal arrives, we
// invoke the callback (if set).

void SigTermHandler::wait_for_sigs()
{
    // TODO[tvoss]: We really should factor this functionality into process-cpp
    // and make it easily reusable.

    static constexpr int signal_fd_idx = 0;
    static constexpr int event_fd_idx = 1;

    static constexpr int signal_info_buffer_size = 5;

    // Better safe than sorry. We *theoretically* should inherit the blocked
    // mask from our parent thread, but it does not hurt setting it up again.
    if (::pthread_sigmask(SIG_BLOCK, &sigs_, nullptr) == -1)
        throw SyscallException(
                "pthread_sigmask failed: " + std::string{std::strerror(errno)},
                errno);

    // Make sure we clean up the signal fd whenever
    // we leave the scope of wait_for_sigs.
    struct Scope
    {
        ~Scope()
        {
            if (signal_fd != -1)
                ::close(signal_fd);
        }

        int signal_fd;
    } scope{::signalfd(-1, &sigs_, SFD_CLOEXEC | SFD_NONBLOCK)};

    if (scope.signal_fd == -1)
    {
        throw SyscallException(
                    "signalfd creation failed" + std::string{std::strerror(errno)},
                    errno);
    }

    pollfd fds[2];
    signalfd_siginfo signal_info[signal_info_buffer_size];

    for (;;)
    {
        fds[signal_fd_idx] = {scope.signal_fd, POLLIN, 0};
        fds[event_fd_idx] = {event_fd_, POLLIN, 0};

        auto rc = ::poll(fds, 2, -1);

        if (rc == -1)
        {
            if (errno == EINTR)
                continue;

            break;
        }

        if (rc == 0)
            continue;

        if (fds[signal_fd_idx].revents & POLLIN)
        {
            auto result = ::read(scope.signal_fd, signal_info, sizeof(signal_info));

            for (uint i = 0; i < result / sizeof(signalfd_siginfo); i++)
            {
                switch (signal_info[i].ssi_signo)
                {
                case SIGINT:
                case SIGHUP:
                case SIGTERM:
                {
                    lock_guard<mutex> lock(mutex_);
                    if (callback_)
                    {
                        callback_();
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }

        if (fds[event_fd_idx].revents & POLLIN)
        {
            std::int64_t value{1};
            // Consciously void-ing the return value here.
            // Not much we can do about an error.
            (void)::read(event_fd_, &value, sizeof(value));
            break;
        }
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
