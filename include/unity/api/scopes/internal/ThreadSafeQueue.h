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

#include <condition_variable>
#include <mutex>
#include <queue>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

// Simple thread-safe queue of items.

template<typename T>
class ThreadSafeQueue final : private util::NonCopyable
{
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() noexcept {};

    void push(T const& item);
    void push(T&& item);
    T wait_and_pop();
    bool try_pop(T&& item);
    bool empty() const noexcept;

private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
};

template<typename T>
inline
void ThreadSafeQueue<T>::push(T const& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(item);
    cond_.notify_one();
}

template<typename T>
inline
void ThreadSafeQueue<T>::push(T&& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.emplace(std::move(item));
    cond_.notify_one();
}

template<typename T>
inline
T ThreadSafeQueue<T>::wait_and_pop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] { return queue_.size() != 0; });
    T item = std::move(queue_.front());
    queue_.pop();
    return item;
}

template<typename T>
inline
bool ThreadSafeQueue<T>::try_pop(T&& item)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty())
    {
        return false;
    }
    item = move(queue_.front());
    queue_.pop();
    return true;
}

template<typename T>
inline
bool ThreadSafeQueue<T>::empty() const noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
