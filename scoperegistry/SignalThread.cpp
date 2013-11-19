/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
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
#include <sys/wait.h>
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
    sigaddset(&sigs_, SIGCHLD);
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
    handler_thread_ = thread([this]{ this->wait_for_sigs(); });
}

SignalThread::~SignalThread() noexcept
{
    {
        lock_guard<mutex> lock(mutex_);
        if (!done_)
        {
            kill(getpid(), SIGUSR1);
        }
    }
    handler_thread_.join();
}

// Add the pid and supplied command line to the table of children.

void SignalThread::add_child(pid_t pid, char const* argv[])
{
    lock_guard<mutex> lock(mutex_);
    if (done_)
    {
        return; // We've been told to go away already
    }
    string args;
    for (int i = 0; argv[i] != nullptr; ++i)
    {
        if (i != 0)
        {
            args.append(" ");
        }
        args.append(argv[i]);
    }
    children_[pid] = args;
}

// Unblock the signals we are handling. This allows the child to reset the signals
// to their defaults before calling exec().

void SignalThread::reset_sigs()
{
    int err = pthread_sigmask(SIG_UNBLOCK, &sigs_, nullptr);
    if (err != 0)
    {
        throw SyscallException("pthread_sigmask failed", err);
    }
}

// Wait for signals. For SIGCHLD, we check exit status and report if a child terminated abnormally.
// The normal termination signals (SIGINT, SIGHUP, and SIGTERM) are sent to the children.
// SIGUSR1 causes the thread to return without touching any of its children.

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
            case SIGCHLD:
            {
                for (;;)
                {
                    // Multiple children exiting can result in only a single SIGCHLD being received,
                    // so we loop until there are no more exited children.
                    int status;
                    pid_t pid = waitpid(-1, &status, WNOHANG);
                    if (pid == -1)
                    {
                        if (errno != ECHILD)
                        {
                            cerr << "scoperegistry: wait() failed: " << strerror(errno) << endl;
                            _exit(1);
                        }
                        continue; // Ignore stray SIGCHLD signals
                    }
                    if (pid == 0)
                    {
                        break;  // No more exited children remain.
                    }

                    lock_guard<mutex> lock(mutex_);
                    auto it = children_.find(pid);
                    if (it == children_.end())
                    {
                        cerr << "scoperegistry: ignoring unexpected child termination, pid = " << pid << endl;
                        continue;
                    }
                    if (WIFEXITED(status))
                    {
                        if (WEXITSTATUS(status) != 0)
                        {
                            cerr << "scoperegistry: process " << pid << " exited with status " << WEXITSTATUS(status)
                                 << ", command line: " << it->second << endl;
                        }
                    }
                    else
                    {
                        cerr << "scoperegistry: process " << pid << " terminated by signal " << WTERMSIG(status)
                             << ", command line: " << it->second << endl;
                    }
                    children_.erase(it);
                }
                break;
            }
            case SIGINT:
            case SIGHUP:
            case SIGTERM:
            {
                // Send the signal to all the children.
                lock_guard<mutex> lock(mutex_);
                for (auto pair : children_)
                {
                    kill(pair.first, signo);
                }
                done_ = true;
                // TODO: for now, until the callback is added, just exit
                _exit(signo);
                break;
            }
            case SIGUSR1:
            {
                cerr << "signal thread: Got SIGUSR1" << endl;
                // TODO: should invoke a callback here that then can shut down the run time.
                return; // Go away without doing anything to the children.
            }
            default:
            {
                assert(false);
            }
        }
    }
}

} // namespace scoperegistry
