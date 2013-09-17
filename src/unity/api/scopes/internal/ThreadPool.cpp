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

#include <unity/api/scopes/internal/ThreadPool.h>

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

ThreadPool::~ThreadPool() noexcept
{
    {
        lock_guard<mutex> lock(mutex_);
        done_ = true;
    }
    cond_.notify_all();
}

void ThreadPool::run()
{
    unique_lock<mutex> lock(mutex_);
    for (;;)
    {
        cond_.wait(lock, [this]{ return done_ || !queue_.empty(); });
        if (done_)
        {
            return;
        }
        auto task = queue_.wait_and_pop();
        task();
    }
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
