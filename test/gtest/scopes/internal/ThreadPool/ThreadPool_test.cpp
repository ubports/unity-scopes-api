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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <valgrind/valgrind.h>

using namespace std;
using namespace unity::scopes::internal;

TEST(ThreadPool, basic)
{
    // Creation and destruction in quick succession
    {
        ThreadPool p(1);
    }
    {
        ThreadPool p(5);
    }
    {
        ThreadPool p(20);
    }
}

atomic_int call_count;

void f()
{
    ++call_count;
    this_thread::sleep_for(chrono::milliseconds(200));
}

void g()
{
    ++call_count;
}

TEST(ThreadPool, submit)
{
    {
        ThreadPool p(1);
        p.submit(f);
        p.submit(f);
    }

    {
        ThreadPool p(5);
        p.submit(f);
        p.submit(f);
    }

    {
        call_count = 0;
        ThreadPool p(20);
        p.submit(f);
        p.submit(f);
        this_thread::sleep_for(chrono::milliseconds(300));
        EXPECT_EQ(2, call_count);
    }

    {
        call_count = 0;
        ThreadPool p(20);
        p.submit(g);
        p.submit(g);
        p.submit(g);
        this_thread::sleep_for(chrono::milliseconds(300));
        EXPECT_EQ(3, call_count);
    }
}

TEST(ThreadPool, exception)
{
    try
    {
        ThreadPool p(0);
        FAIL();
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ThreadPool(): invalid pool size: 0", e.what());
    }

    try
    {
        ThreadPool p(-1);
        FAIL();
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ThreadPool(): invalid pool size: -1", e.what());
    }

    try
    {
        ThreadPool p(1);
        p.destroy();
        p.submit([]{});
        FAIL();
    }
    catch (std::runtime_error const& e)
    {
        EXPECT_STREQ("ThreadPool::submit(): cannot accept task for destroyed pool", e.what());
    }

   // Valgrind can't handle the attempt to create that many threads.
   // Address sanitizer survives, but takes more than a minute.
#if defined(__has_feature)
    #if !__has_feature(address_sanitizer)

    if (!RUNNING_ON_VALGRIND)
    {
        try
        {
            ThreadPool p(100000000);  // A hundred million threads is impossible
            FAIL();
        }
        catch (unity::ResourceException const& e)
        {
            EXPECT_STREQ("unity::ResourceException: ThreadPool(): exception during pool creation:\n"
                         "    Resource temporarily unavailable",
                         e.what());
        }
    }

    #endif
#endif
}

TEST(ThreadPool, throwing_task)
{
    ThreadPool p(1);
    auto throw_std_ex = [](){ throw std::logic_error("some error"); };
    auto throw_int = [](){ throw 99; };
    auto f_ex = p.submit(throw_std_ex);
    auto f_int = p.submit(throw_int);
    EXPECT_THROW(f_ex.get(), std::logic_error);
    EXPECT_THROW(f_int.get(), int);
}

TEST(ThreadPool, destroy)
{
    ThreadPool p(1);

    auto fut = std::async(launch::async, [&p] {
        this_thread::sleep_for(chrono::milliseconds(100));
        p.destroy();
    });
    p.wait_for_destroy();  // OK to wait after destroy, returns immediately
    fut.wait();

    p.destroy();           // OK to destroy if already destroyed
    p.destroy();
}

TEST(ThreadPool, wait_for_destroy)
{
    ThreadPool p(1);

    auto fut = std::async(launch::async, [&p] { p.destroy(); });
    p.wait_for_destroy();
    p.wait_for_destroy();    // OK to wait again, waits until previous call completes
    p.destroy();             // OK to destroy if already destroyed
    p.wait_for_destroy();    // OK to wait again, returns immediately
    p.destroy_once_empty();  // OK to destroy again, returns immediately
    p.wait_for_destroy();    // OK to wait again, returns immediately
    fut.wait();
}

void exercise_pool(int num_threads, int num_tasks, int delay_ms)
{
    ThreadPool p(num_threads);

    auto slow_task = [delay_ms] { this_thread::sleep_for(chrono::milliseconds(delay_ms)); };

    for (int i = 0; i < num_tasks; ++i)
    {
        p.submit(slow_task);    // num_tasks taking delay_ms each
    }
    p.destroy_once_empty();
}

TEST(ThreadPool, destroy_once_empty)
{
    const int num_tasks = 6;
    const int delay_ms = 200;

    // num_tasks run by a single thread
    {
        auto start_time = chrono::system_clock::now();
        exercise_pool(1, num_tasks, delay_ms);
        auto end_time = chrono::system_clock::now();
        auto millisecs = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
        EXPECT_LT(millisecs, num_tasks * delay_ms + delay_ms);  // Allow delay_ms margin
    }

    // num_tasks run by num_task threads
    {
        auto start_time = chrono::system_clock::now();
        exercise_pool(num_tasks, num_tasks, delay_ms);
        auto end_time = chrono::system_clock::now();
        auto millisecs = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
        EXPECT_LT(millisecs, delay_ms + delay_ms);  // Allow delay_ms margin
    }

    // num_tasks run by > num_task threads
    {
        auto start_time = chrono::system_clock::now();
        exercise_pool(num_tasks + 1, num_tasks, delay_ms);
        auto end_time = chrono::system_clock::now();
        auto millisecs = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count();
        EXPECT_LT(millisecs, delay_ms + delay_ms);  // Allow delay_ms margin
    }
}

TEST(ThreadPool, destroy_once_empty_while_waiting)
{
    ThreadPool p(1);

    auto slow_task = []{ this_thread::sleep_for(chrono::milliseconds(300)); };
    p.submit(slow_task);
    // No test here, just to get coverage on the Waiting state.
    auto fut = std::async(launch::async, [&p] { p.destroy_once_empty(); });
    auto fut2 = std::async(launch::async, [&p] { p.destroy_once_empty(); });
    fut.wait();
    fut2.wait();
    p.wait_for_destroy();
}

TEST(ThreadPool, destroy_while_waiting)
{
    ThreadPool p(1);

    auto slow_task = []{ this_thread::sleep_for(chrono::milliseconds(300)); };
    p.submit(slow_task);
    // No test here, just to get coverage on the Waiting state.
    auto fut = std::async(launch::async, [&p] { p.destroy(); });
    auto fut2 = std::async(launch::async, [&p] { p.destroy(); });
    fut.wait();
    fut2.wait();
    p.wait_for_destroy();
}
