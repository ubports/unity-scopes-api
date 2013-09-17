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

#ifndef UNITY_API_SCOPES_INTERNAL_THREADPOOL_H
#define UNITY_API_SCOPES_INTERNAL_THREADPOOL_H

#include <unity/api/scopes/internal/ThreadSafeQueue.h>
#include <unity/api/scopes/internal/TaskWrapper.h>
#include <unity/UnityExceptions.h>

#include <future>
#include <thread>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// Simple thread pool that runs tasks on a number of worker threads.
// submit() accepts an arbitrary functor and returns a future that
// the calling thread can use to wait for the task to complete.

class ThreadPool final : private util::NonCopyable
{
public:
    template<typename StartFunc>
    ThreadPool(int size, StartFunc func);
    ~ThreadPool() noexcept;

    template<typename F>
    std::future<typename std::result_of<F()>::type> submit(F f);

    void run();

private:
    ThreadSafeQueue<unity::api::scopes::internal::TaskWrapper> queue_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool done_;
};

template<typename StartFunc>
ThreadPool::ThreadPool(int size, StartFunc func)
    : done_(false)
{
    if (size < 1)
    {
        throw InvalidArgumentException("ThreadPool(): invalid pool size: " + std::to_string(size));
    }

    try
    {
        for (int i = 0; i < size; ++i)
        {
            threads_.push_back(std::thread(func, this));
        }
    }
    catch (...)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            done_ = true;
        }
        cond_.notify_all();
        throw;
    }
}

template<typename F>
std::future<typename std::result_of<F()>::type> ThreadPool::submit(F f)
{
    typedef typename std::result_of<F()>::type ResultType;

    std::packaged_task<ResultType()> task(std::move(f));
    std::future<ResultType> result(task.get_future());
    queue_.push(move(task));
    return result;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
