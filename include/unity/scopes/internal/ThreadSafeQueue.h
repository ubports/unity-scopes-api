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

#ifndef UNITY_SCOPES_INTERNAL_THREADSAFEQUEUE_H
#define UNITY_SCOPES_INTERNAL_THREADSAFEQUEUE_H

#include <unity/util/NonCopyable.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>

namespace unity
{

namespace scopes
{

namespace internal
{

// Simple thread-safe queue of items.
// If the queue is destroyed while threads are blocked in wait_and_pop(), wait_and_pop() throws std::runtime_error.

template<typename T>
class ThreadSafeQueue final
{
public:
    NONCOPYABLE(ThreadSafeQueue);
    typedef T value_type;

    ThreadSafeQueue();
    ~ThreadSafeQueue() noexcept;

    void destroy() noexcept;
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
    std::atomic<int> num_waiters_;
};

template<typename T>
ThreadSafeQueue<T>::ThreadSafeQueue() :
    done_(false),
    num_waiters_(0)
{
}

template<typename T>
ThreadSafeQueue<T>::~ThreadSafeQueue() noexcept
{
    destroy();

    // Don't destroy the object while there are still threads in wait_and_pop(), otherwise
    // a thread that wakes up in wait_and_pop() will try to re-lock the already-destroyed
    // mutex.
    while (num_waiters_.load() > 0)
        std::this_thread::yield();  // LCOV_EXCL_LINE (impossible to reliably hit with a test)
}

template<typename T>
void ThreadSafeQueue<T>::destroy() noexcept
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (done_)
    {
        return;
    }
    done_ = true;
    cond_.notify_all(); // Wake up anyone asleep in wait_and_pop()
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
    ++num_waiters_;
    cond_.wait(lock, [this] { return done_ || queue_.size() != 0; });
    if (done_)
    {
        lock.unlock();
        --num_waiters_;
        throw std::runtime_error("ThreadSafeQueue: queue destroyed while thread was blocked in wait_and_pop()");
    }
    T item = std::move(queue_.front());
    queue_.pop();
    --num_waiters_;
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

} // namespace unity

#endif
