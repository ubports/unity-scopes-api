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

#include <scopes/internal/ThreadSafeQueue.h>

#include <gtest/gtest.h>

#include <future>

using namespace std;
using namespace unity::api::scopes::internal;

TEST(ThreadSafeQueue, basic)
{
    ThreadSafeQueue<int> q;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(0, q.size());
    int n;
    EXPECT_FALSE(q.try_pop(n));

    q.push(5);                // R-value
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(1, q.size());
    n = q.wait_and_pop();
    EXPECT_EQ(5, n);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(0, q.size());

    n = 6;                    // L-value
    q.push(n);
    EXPECT_EQ(1, q.size());
    auto r = q.wait_and_pop();
    EXPECT_EQ(6, r);
    EXPECT_EQ(0, q.size());

    n = 7;
    q.push(n);
    EXPECT_EQ(1, q.size());
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
    unique_ptr<ThreadSafeQueue<string>> q(new ThreadSafeQueue<string>);
    q->push("fred");
    auto f = waiter_ready.get_future();
    auto t = thread(&waiter_thread, q.get());
    f.wait();
    this_thread::sleep_for(chrono::milliseconds(50));   // Make sure child thread has time to call wait_and_pop()
    q.reset();
    t.join();
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
}
