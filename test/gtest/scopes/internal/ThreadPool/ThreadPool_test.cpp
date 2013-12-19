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

#include <scopes/internal/ThreadPool.h>

#include <gtest/gtest.h>

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

atomic_int count;

void f()
{
    ++count;
    this_thread::sleep_for(chrono::milliseconds(200));
}

void g()
{
    ++count;
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
        count = 0;
        ThreadPool p(20);
        p.submit(f);
        p.submit(f);
        this_thread::sleep_for(chrono::milliseconds(300));
        EXPECT_EQ(2, count);
    }

    {
        count = 0;
        ThreadPool p(20);
        p.submit(g);
        p.submit(g);
        p.submit(g);
        this_thread::sleep_for(chrono::milliseconds(300));
        EXPECT_EQ(3, count);
    }
}

TEST(ThreadPool, exception)
{
    try
    {
        ThreadPool p(0);
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ThreadPool(): invalid pool size: 0", e.to_string());
    }

    try
    {
        ThreadPool p(-1);
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ThreadPool(): invalid pool size: -1", e.to_string());
    }
}
