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

#include <SignalThread.h>

#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream>

#include <signal.h>
#include <string.h>
#include <unistd.h>

using namespace std;
using namespace unity;

namespace scoperegistry
{

SignalThread::SignalThread() :
    done_(false)
{
    // Block signals in the caller, so they are delivered to the thread we are about to create.
    sigemptyset(&sigs_);
    sigaddset(&sigs_, SIGINT);
    sigaddset(&sigs_, SIGHUP);
    sigaddset(&sigs_, SIGTERM);
    sigaddset(&sigs_, SIGUSR1);
    int err = pthread_sigmask(SIG_BLOCK, &sigs_, nullptr);
    if (err != 0)
    {
        throw SyscallException("pthread_sigmask failed", err);
    }

    // Make ourselves a process group leader.
    setpgid(0, 0);

    // Run a signal handling thread that waits for any of the above signals.
    lock_guard<mutex> lock(mutex_);
    handler_thread_ = thread([this]{ this->wait_for_sigs(); });
}

SignalThread::~SignalThread()
{
    stop();
    handler_thread_.join();
}

void SignalThread::activate(function<void()> callback)
{
    lock_guard<mutex> lock(mutex_);
    callback_ = callback;
}

void SignalThread::stop()
{
    lock_guard<mutex> lock(mutex_);
    if (!done_)
    {
        done_ = true;
        kill(getpid(), SIGUSR1);
    }
}

// Wait for termination signals. When a termination signal arrives, we
// invoke the callback (if set). If we receive SIGUSR1 (because
// we are terminating ourself, we exit the thread.

void SignalThread::wait_for_sigs()
{
    int signo;
    for (;;)
    {
        int err = sigwait(&sigs_, &signo);
        if (err != 0)
        {
            cerr << "scoperegistry: sigwait failed: " << strerror(err) << endl;
            _exit(1);
        }
        switch (signo)
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
            case SIGUSR1:
            {
                return;
            }
            default:
            {
                assert(false);
            }
        }
    }
}

} // namespace scoperegistry
