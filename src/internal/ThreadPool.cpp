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

#include <scopes/internal/ThreadPool.h>

#include <unity/UnityExceptions.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ThreadPool::ThreadPool(int size)
{
    if (size < 1)
    {
        throw InvalidArgumentException("ThreadPool(): invalid pool size: " + std::to_string(size));
    }

    queue_.reset(new TaskQueue);

    try
    {
        for (int i = 0; i < size; ++i)
        {
            threads_.push_back(std::thread(&ThreadPool::run, this));
        }
    }
    catch (std::exception const&)   // LCOV_EXCL_LINE
    {
        throw ResourceException("ThreadPool(): exception during pool creation");  // LCOV_EXCL_LINE
    }
}

ThreadPool::~ThreadPool() noexcept
{
    queue_->destroy();
    try
    {
        for (size_t i = 0; i < threads_.size(); ++i)
        {
            threads_[i].join();
        }
    }
    catch (...) // LCOV_EXCL_LINE
    {
        assert(false);  // LCOV_EXCL_LINE
    }
}

void ThreadPool::run()
{
    TaskQueue::value_type task;
    for (;;)
    {
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

} // namespace api

} // namespace unity
