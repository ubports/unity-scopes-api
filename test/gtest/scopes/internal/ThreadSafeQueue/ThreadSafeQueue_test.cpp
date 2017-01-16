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

#include <unity/scopes/internal/ThreadSafeQueue.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <future>

using namespace std;
using namespace unity::scopes::internal;

TEST(ThreadSafeQueue, basic)
{
    ThreadSafeQueue<int> q;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(0u, q.size());
    int n;
    EXPECT_FALSE(q.try_pop(n));

    q.push(5);                // R-value
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(1u, q.size());
    n = q.wait_and_pop();
    EXPECT_EQ(5, n);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(0u, q.size());

    n = 6;                    // L-value
    q.push(n);
    EXPECT_EQ(1u, q.size());
    auto r = q.wait_and_pop();
    EXPECT_EQ(6, r);
    EXPECT_EQ(0u, q.size());

    n = 7;
    q.push(n);
    EXPECT_EQ(1u, q.size());
    EXPECT_TRUE(q.try_pop(r));
    EXPECT_EQ(7, r);
}

promise<void> waiter_ready;

void waiter_thread(ThreadSafeQueue<string>* q)
{
    EXPECT_EQ("fred", q->wait_and_pop());
    waiter_ready.set_value();
    try
    {
        q->wait_and_pop();
        FAIL();
    }
    catch (std::runtime_error const& e)
    {
        EXPECT_STREQ("ThreadSafeQueue: queue destroyed while thread was blocked in wait_and_pop()", e.what());
    }
}

TEST(ThreadSafeQueue, exception)
{
    {
        unique_ptr<ThreadSafeQueue<string>> q(new ThreadSafeQueue<string>);
        q->push("fred");
        auto f = waiter_ready.get_future();
        auto t = thread(waiter_thread, q.get());
        f.wait();
        this_thread::sleep_for(chrono::milliseconds(50));   // Make sure child thread has time to call wait_and_pop()
        q->destroy();
        t.join();

        try
        {
            q->push("fred");    // Move push
            FAIL();
        }
        catch (std::runtime_error const& e)
        {
            EXPECT_STREQ("ThreadSafeQueue: cannot push onto destroyed queue", e.what());
        }

        try
        {
            string s = "fred";
            q->push(s);         // Copy push
            FAIL();
        }
        catch (std::runtime_error const& e)
        {
            EXPECT_STREQ("ThreadSafeQueue: cannot push onto destroyed queue", e.what());
        }
    }
}

atomic_int call_count;

void int_reader_thread(ThreadSafeQueue<int>* q)
{
    try
    {
        q->wait_and_pop();
        FAIL();
    }
    catch (std::runtime_error const&)
    {
        ++call_count;
    }
}

TEST(ThreadSafeQueue, wait_for_threads)
{
    ThreadSafeQueue<int> q;
    call_count = 0;
    vector<thread> threads;
    for (auto i = 0; i < 20; ++i)
    {
        threads.push_back(thread(int_reader_thread, &q));
    }
    this_thread::sleep_for(chrono::milliseconds(300));

    // Destroy the queue while multiple threads are sleeping in wait_and_pop().
    q.destroy();
    q.wait_for_destroy();

    for (auto& t : threads)
    {
        t.join();
    }
    EXPECT_EQ(20, call_count);
}

void destroy_thread(ThreadSafeQueue<int>* q)
{
    this_thread::sleep_for(chrono::milliseconds(100));
    EXPECT_EQ(0u, q->size());
    q->destroy();
}

TEST(ThreadSafeQueue, destroy_while_empty)
{
    ThreadSafeQueue<int> q;
    thread t(destroy_thread, &q);
    q.wait_for_destroy();
    EXPECT_EQ(0u, q.size());
    t.join();
}

void wait_for_destroy_thread(ThreadSafeQueue<int>* q)
{
    q->wait_for_destroy();
}

TEST(ThreadSafeQueue, destroy_while_waiting_for_destroy)
{
    ThreadSafeQueue<int> q;
    q.push(42);
    thread t(wait_for_destroy_thread, &q);
    this_thread::sleep_for(chrono::milliseconds(100));
    q.destroy();
    t.join();

    // Call destroy() and wait_for_destroy() again, to make sure they do nothing.
    q.destroy();
    q.wait_for_destroy();
    q.destroy();
    q.destroy();
    q.wait_for_destroy();
    q.wait_for_destroy();
}

class MoveOnly
{
public:
    MoveOnly(string s) :
        s_(s)
    {
    }

    MoveOnly(MoveOnly const&) = delete;
    MoveOnly& operator=(MoveOnly const&) = delete;

    MoveOnly(MoveOnly&& other)  = default;
    MoveOnly& operator=(MoveOnly&& rhs) = default;

    string const& val() { return s_; }

private:
    string s_;
};

TEST(ThreadSafeQueue, move_only)
{
    ThreadSafeQueue<MoveOnly> q;

    q.push(move(MoveOnly("hello")));
    q.push(move(MoveOnly("world")));
    q.push(move(MoveOnly("again")));

    MoveOnly m("");

    EXPECT_TRUE(q.try_pop(m));
    EXPECT_EQ("hello", m.val());
    EXPECT_TRUE(q.try_pop(m));
    EXPECT_EQ("world", m.val());
    m = q.wait_and_pop();
    EXPECT_EQ("again", m.val());

    q.destroy();

    try
    {
        q.push(move(MoveOnly("no_push")));
        FAIL();
    }
    catch (std::runtime_error const& e)
    {
        EXPECT_STREQ("ThreadSafeQueue: cannot push onto destroyed queue", e.what());
    }
}

TEST(ThreadSafeQueue, wait_until_empty)
{
    ThreadSafeQueue<int> q;
    q.push(99);
    auto fut = std::async(launch::async, [&q] {
        this_thread::sleep_for(chrono::milliseconds(300)); q.wait_and_pop(); q.destroy();
    });
    q.wait_until_empty();
    EXPECT_TRUE(q.empty());
    fut.wait();
}
