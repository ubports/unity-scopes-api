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

#ifndef UNITY_API_SCOPES_INTERNAL_THREADSAFEQUEUE_H
#define UNITY_API_SCOPES_INTERNAL_THREADSAFEQUEUE_H

#include <unity/util/NonCopyable.h>

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// Simple thread-safe queue of items.
// If the queue is destroyed while threads are blocked in wait_and_pop(), wait_and_pop() throws std::runtime_error.

template<typename T>
class ThreadSafeQueue final : private util::NonCopyable
{
public:
    typedef T value_type;

    ThreadSafeQueue();
    ~ThreadSafeQueue() noexcept;

    void push(T const& item);
    void push(T&& item);
    T wait_and_pop();
    bool try_pop(T& item);
    bool empty() const noexcept;
    size_t size() const noexcept;

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    bool done_;
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue() :
    done_(false)
{
}

template<typename T>
ThreadSafeQueue<T>::~ThreadSafeQueue() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    done_ = true;
    cond_.notify_all();
}

template<typename T>
void ThreadSafeQueue<T>::push(T const& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(item);
    cond_.notify_one();
}

template<typename T>
void ThreadSafeQueue<T>::push(T&& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(std::move(item));
    cond_.notify_one();
}

template<typename T>
T ThreadSafeQueue<T>::wait_and_pop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return done_ || queue_.size() != 0; });
    if (done_)
    {
        throw std::runtime_error("ThreadSafeQueue: queue destroyed while thread was blocked in wait_and_pop()");
    }
    T item = std::move(queue_.front());
    queue_.pop();
    return item;
}

template<typename T>
bool ThreadSafeQueue<T>::try_pop(T& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
    {
        return false;
    }
    item = std::move(queue_.front());
    queue_.pop();
    return true;
}

template<typename T>
bool ThreadSafeQueue<T>::empty() const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template<typename T>
size_t ThreadSafeQueue<T>::size() const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
