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

#ifndef UNITY_SCOPES_INTERNAL_THREADPOOL_H
#define UNITY_SCOPES_INTERNAL_THREADPOOL_H

#include <unity/scopes/internal/ThreadSafeQueue.h>
#include <unity/scopes/internal/TaskWrapper.h>

#include <future>

namespace unity
{

namespace scopes
{

namespace internal
{

// Simple thread pool that runs tasks on a number of worker threads.
// submit() accepts an arbitrary functor and returns a future that
// the calling thread can use to wait for the task to complete.

class ThreadPool final
{
public:
    NONCOPYABLE(ThreadPool);
    UNITY_DEFINES_PTRS(ThreadPool);

    ThreadPool(int num_threads);         // Create pool with specified number of threads
    ~ThreadPool();

    void destroy() noexcept;             // Destroys whether queue is empty or not; waits for threads to exit.
    void destroy_once_empty() noexcept;  // Waits for queue to become empty, then calls destroy().
    void wait_for_destroy() noexcept;    // Blocks until destruction is complete.

    template<typename F>
    std::future<typename std::result_of<F()>::type> submit(F f);  // Pushes processing task onto queue.

private:
    void run();

    typedef ThreadSafeQueue<unity::scopes::internal::TaskWrapper> TaskQueue;
    std::unique_ptr<TaskQueue> queue_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::condition_variable cond_;
    enum State { Created, Waiting, Destroying, Destroyed };
    State state_;
};

template<typename F>
std::future<typename std::result_of<F()>::type> ThreadPool::submit(F f)
{
    typedef typename std::result_of<F()>::type ResultType;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ != Created)
        {
            throw std::runtime_error("ThreadPool::submit(): cannot accept task for destroyed pool");
        }
    }
    std::packaged_task<ResultType()> task(std::move(f));
    std::future<ResultType> result(task.get_future());
    queue_->push(move(task));
    return result;
}

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
