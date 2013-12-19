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

#include <scopes/internal/Reaper.h>

#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class Counter
{
public:
    Counter() : c(0) {}
    void reset() { lock_guard<mutex> l(m); c = 0; }
    int get() { lock_guard<mutex> l(m); return c; }
    void increment() { lock_guard<mutex> l(m); ++c; }
    void increment_throw() { lock_guard<mutex> l(m); ++c; throw 42; }

private:
    int c;
    mutex m;
};

// Basic tests.

TEST(Reaper, basic)
{
    {
        Reaper::create(1, 5);   // Just to check that we can create and destroy without doing anything else
    }

    {
        Reaper::create(1, 5, Reaper::CallbackOnDestroy);   // Again, with the other policy
    }

    {
        // Make sure it's OK for the reaper to go out of scope while there are still ReapItems on it.
        // Check that none of the callbacks are called in this case.
        Counter c;
        {
            auto r = Reaper::create(5, 5);
            for (auto i = 0; i < 10; ++i)
            {
                r->add(bind(&Counter::increment, &c));
            }
            EXPECT_EQ(10, r->size());
        }
        EXPECT_EQ(0, c.get());
    }

    {
        // Same again, but with CallbackOnDestroy.
        // Check that all of the callbacks are called in this case.
        Counter c;
        {
            auto r = Reaper::create(5, 5, Reaper::CallbackOnDestroy);
            for (auto i = 0; i < 10; ++i)
            {
                r->add(bind(&Counter::increment, &c));
            }
            EXPECT_EQ(10, r->size());
        }
        EXPECT_EQ(10, c.get());
    }

    {
        // Make sure it's OK to invoke methods on a ReapItem after the reaper has gone out of scope.
        Counter c;
        vector<ReapItem::SPtr> v;
        {
            auto r = Reaper::create(5, 5);
            for (auto i = 0; i < 10; ++i)
            {
                v.push_back(r->add(bind(&Counter::increment, &c)));
            }
            EXPECT_EQ(10, r->size());
        }
        EXPECT_EQ(0, c.get());

        v[0]->refresh(); // Does nothing
        v[1]->destroy(); // Does nothing
        v[1]->destroy(); // Safe to call more than once
        v[1]->refresh(); // Safe to call after destroy
    }

    {
        // Make sure that calling destroy on a ReapItem removes that item from the reaper.
        Counter c;
        vector<ReapItem::SPtr> v;
        {
            auto r = Reaper::create(5, 5);
            for (auto i = 0; i < 10; ++i)
            {
                v.push_back(r->add(bind(&Counter::increment, &c)));
            }
            EXPECT_EQ(10, r->size());
            v[0]->destroy();
            v[4]->destroy();
            v[9]->destroy();
            EXPECT_EQ(7, r->size());
            // We call destroy again, to make sure that it's safe to call it twice even though the first time
            // around, the destroy actually removed the item.
            v[0]->destroy();
            v[4]->destroy();
            v[9]->destroy();
            EXPECT_EQ(7, r->size());
        }
        EXPECT_EQ(0, c.get());
    }

    {
        // Make sure that, after refreshing an item, it still can be destroyed, that is, that the ReapItem
        // is correctly located in the reaper list even after having been moved.
        Counter c;
        vector<ReapItem::SPtr> v;
        {
            auto r = Reaper::create(5, 5);
            v.push_back(r->add(bind(&Counter::increment, &c)));
            v.push_back(r->add(bind(&Counter::increment, &c)));
            v.push_back(r->add(bind(&Counter::increment, &c)));
            EXPECT_EQ(3, r->size());

            v[0]->refresh(); // Moves this element from the tail to the head
            EXPECT_EQ(3, r->size());
            v[0]->destroy();
            EXPECT_EQ(2, r->size());
            v[0]->destroy();            // no-op
            EXPECT_EQ(2, r->size());
            v[1]->destroy();
            v[2]->destroy();
            EXPECT_EQ(0, r->size());
        }
        EXPECT_EQ(0, c.get());
    }
}

TEST(Reaper, expiry)
{
    {
        Counter c;
        auto r = Reaper::create(1, 2);

        // Entries expire after 2 seconds.
        auto e1 = r->add(bind(&Counter::increment, &c));
        auto e2 = r->add(bind(&Counter::increment, &c));

        // One second later, they still must both be there.
        this_thread::sleep_for(chrono::milliseconds(1000));
        EXPECT_EQ(2, r->size());
        EXPECT_EQ(0, c.get());

        // Refresh one of the entries.
        e2->refresh();

        // 1.2 seconds later, one of them must have disappeared.
        this_thread::sleep_for(chrono::milliseconds(1200));
        EXPECT_EQ(1, r->size());
        EXPECT_EQ(1, c.get());

        // 0.6 seconds later, the second entry must still be around.
        this_thread::sleep_for(chrono::milliseconds(600));
        EXPECT_EQ(1, r->size());
        EXPECT_EQ(1, c.get());

        // 0.4 seconds later, the second entry must have disappeared.
        this_thread::sleep_for(chrono::milliseconds(400));
        EXPECT_EQ(0, r->size());
        EXPECT_EQ(2, c.get());
    }
}

TEST(Reaper, exceptions)
{
    try
    {
        Reaper::create(0, 5);
        FAIL();
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: Reaper: invalid reap_interval (0). Interval must be > 0.",
                  e.to_string());
    }

    try
    {
        Reaper::create(2, 1);
        FAIL();
    }
    catch (unity::LogicException const& e)
    {
        EXPECT_EQ("unity::LogicException: Reaper: reap_interval (2) must be <= expiry_interval (1).",
                  e.to_string());
    }

    try
    {
        auto r = Reaper::create(1, 2);
        r->add(std::function<void()>());
        FAIL();
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: Reaper: invalid null callback passed to add().",
                  e.to_string());
    }

    // Make sure that if a callback throws an exception, this does no harm.
    Counter c;
    {
        auto r = Reaper::create(5, 5, Reaper::CallbackOnDestroy);
        r->add(bind(&Counter::increment_throw, &c));
        EXPECT_EQ(1, r->size());
        EXPECT_EQ(0, c.get());
    }
    EXPECT_EQ(1, c.get());
}
