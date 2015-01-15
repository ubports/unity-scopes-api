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

#include <unity/scopes/internal/ThreadPool.h>

#include <unity/UnityExceptions.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ThreadPool::ThreadPool(int num_threads)
    : queue_(new TaskQueue)
    , state_(Created)
{
    if (num_threads < 1)
    {
        throw InvalidArgumentException("ThreadPool(): invalid pool size: " + std::to_string(num_threads));
    }

    try
    {
        for (int i = 0; i < num_threads; ++i)
        {
            threads_.push_back(std::thread(&ThreadPool::run, this));
        }
    }
    catch (...)
    {
        queue_->destroy();          // Causes any threads that were created to exit.
        for (auto&& t : threads_)
        {
            t.join();
        }
        throw ResourceException("ThreadPool(): exception during pool creation");
    }
}

ThreadPool::~ThreadPool()
{
    destroy();
}

void ThreadPool::destroy() noexcept
{
    vector<thread> threads;

    {
        unique_lock<mutex> lock(mutex_);
        switch (state_)
        {
            case Destroyed:
            {
                return;  // Already destroyed, no-op.
            }
            case Destroying:
            {
                // Block until some other thread has finished destruction.
                cond_.wait(lock, [this]{ return state_ == Destroyed; });
                return;
            }
            default:
            {
                assert(state_ == Created || state_ == Waiting);
                state_ = Destroying;
                // No notify here because no-one waits for Destroying.

                queue_->destroy();
                threads.swap(threads_);
            }
        }
    }

    // Join with threads with the lock released.
    for (size_t i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }

    lock_guard<mutex> lock(mutex_);
    state_ = Destroyed;
    cond_.notify_all();              // Wake up everyone else waiting for destruction to complete.
}

void ThreadPool::destroy_once_empty() noexcept
{
    unique_lock<mutex> lock(mutex_);
    switch (state_)
    {
        case Destroyed:
        {
            return;  // Nothing to do.
        }
        case Created:
        {
            state_ = Waiting;
            lock.unlock();               // Release lock while waiting for queue to drain.
            queue_->wait_until_empty();
            destroy();
            return;
        }
        default:
        {
            assert(state_ == Waiting || state_ == Destroying);
            cond_.wait(lock, [this]{ return state_ == Destroyed; });
            return;
        }
    }
    // NOTREACHED
}

void ThreadPool::wait_for_destroy() noexcept
{
    unique_lock<mutex> lock(mutex_);
    cond_.wait(lock, [this]{ return state_ == Destroyed; });
}

void ThreadPool::run()
{
    for (;;)
    {
        TaskQueue::value_type task;  // Task must go out of scope in each iteration, in case it stores shared_ptrs.
        try
        {
            task = queue_->wait_and_pop();
        }
        catch (runtime_error const&)
        {
            return; // wait_and_pop() throws if the queue is destroyed while threads are blocked on it.
        }
        task();
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
