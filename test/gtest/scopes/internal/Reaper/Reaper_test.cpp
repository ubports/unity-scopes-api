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

#include <unity/scopes/internal/Reaper.h>

#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class Counter
{
public:
    Counter(int delay = 0) : c(0), d(chrono::milliseconds(delay)) {}
    void reset() { c = 0; }
    int get() { return c; }
    void increment() { ++c; this_thread::sleep_for(d); }
    void increment_throw() { ++c; throw 42; }

private:
    atomic_int c;
    chrono::milliseconds d;
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
        vector<ReapItem::SPtr> v;
        {
            auto r = Reaper::create(5, 5);
            for (auto i = 0; i < 10; ++i)
            {
                v.push_back(r->add(bind(&Counter::increment, &c)));
            }
            EXPECT_EQ(10u, r->size());
        }
        EXPECT_EQ(0, c.get());
    }

    {
        // Same again, but with CallbackOnDestroy.
        // Check that all of the callbacks are called in this case.
        Counter c;
        vector<ReapItem::SPtr> v;
        {
            auto r = Reaper::create(5, 5, Reaper::CallbackOnDestroy);
            for (auto i = 0; i < 10; ++i)
            {
                v.push_back(r->add(bind(&Counter::increment, &c)));
            }
            EXPECT_EQ(10u, r->size());
        }
        EXPECT_EQ(10, c.get());
    }

    {
        // Same again, but letting each ReapItem go out of scope immediately.
        // Check that none of the callbacks are called in this case.
        Counter c;
        {
            auto r = Reaper::create(5, 5, Reaper::CallbackOnDestroy);
            for (auto i = 0; i < 10; ++i)
            {
                r->add(bind(&Counter::increment, &c));
            }
            EXPECT_EQ(0u, r->size());
        }
        EXPECT_EQ(0, c.get());
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
            EXPECT_EQ(10u, r->size());
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
            EXPECT_EQ(10u, r->size());
            v[0]->destroy();
            v[4]->destroy();
            v[9]->destroy();
            EXPECT_EQ(7u, r->size());
            // We call destroy again, to make sure that it's safe to call it twice even though the first time
            // around, the destroy actually removed the item.
            v[0]->destroy();
            v[4]->destroy();
            v[9]->destroy();
            EXPECT_EQ(7u, r->size());
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
            EXPECT_EQ(3u, r->size());

            v[0]->refresh(); // Moves this element from the tail to the head
            EXPECT_EQ(3u, r->size());
            v[0]->destroy();
            EXPECT_EQ(2u, r->size());
            v[0]->destroy();            // no-op
            EXPECT_EQ(2u, r->size());
            v[1]->destroy();
            v[2]->destroy();
            EXPECT_EQ(0u, r->size());
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
        EXPECT_EQ(2u, r->size());
        EXPECT_EQ(0, c.get());

        // Refresh one of the entries.
        e2->refresh();

        // 1.2 seconds later, one of them must have disappeared.
        this_thread::sleep_for(chrono::milliseconds(1200));
        EXPECT_EQ(1u, r->size());
        EXPECT_EQ(1, c.get());

        // 0.6 seconds later, the second entry must still be around.
        this_thread::sleep_for(chrono::milliseconds(600));
        EXPECT_EQ(1u, r->size());
        EXPECT_EQ(1, c.get());

        // 0.4 seconds later, the second entry must have disappeared.
        this_thread::sleep_for(chrono::milliseconds(400));
        EXPECT_EQ(0u, r->size());
        EXPECT_EQ(2, c.get());
    }

    {
        auto r = Reaper::create(2, 3, Reaper::CallbackOnDestroy);

        // Sleep for 0.5 seconds, so the reaper is blocked,
        // waiting for something to be added to the queue.
        this_thread::sleep_for(chrono::milliseconds(500));

        // Add three entries. This wakes up the reaper thread,
        // which schedules the next reaping pass 3 seconds
        // from now.
        // t == 0
        Counter c1;
        auto e1 = r->add(bind(&Counter::increment, &c1));
        Counter c2;
        auto e2 = r->add(bind(&Counter::increment, &c2));
        Counter c3;
        auto e3 = r->add(bind(&Counter::increment, &c3));

        // Wait 1.5 seconds and refresh e1.
        this_thread::sleep_for(chrono::milliseconds(1500));
        e1->refresh();

        // t == 1.5
        // Remaining life times:
        // e1: 3.0
        // e2: 1.5
        // e3: 1.5

        // Wait for 1 second and refresh e2.
        this_thread::sleep_for(chrono::seconds(1));
        e2->refresh();

        // t == 2.5
        // Remaining life times:
        // e1: 2.0
        // e2: 3.0
        // e3: 0.5
        //
        // Wait for 1 second. While we are asleep, the first
        // Reaping pass happens.
        this_thread::sleep_for(chrono::seconds(1));  // t == 3.0, first reaping pass, next pass at t == 5.0

        // t == 3.5
        // While we were asleep, the first pass must have reaped e3.
        EXPECT_EQ(2u, r->size());
        EXPECT_EQ(0, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(1, c3.get());

        // t == 3.5
        // Remaining life times:
        // e1: 1.0
        // e2: 2.0

        // The first reaping pass happened 0.5 seconds ago, and the next
        // pass won't happen for another 1.5 seconds, because we reap
        // at most once every 2 seconds.
        // We sleep another 1.25 seconds, which expires e1.
        this_thread::sleep_for(chrono::milliseconds(1250));

        // Remaining life times:
        // e1: -0.25 (expired)
        // e2: 0.75
        EXPECT_EQ(2u, r->size());  // e1 is still there because it is expired, but not reaped.
        EXPECT_EQ(0, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(1, c3.get());

        // We destroy e2, which cancels the callback for it.
        e2 = nullptr;
        EXPECT_EQ(1u, r->size());  // e1 is still there because it is expired, but not reaped.
        EXPECT_EQ(0, c1.get());
        EXPECT_EQ(0, c2.get());    // Callback for e2 must not have been invoked.
        EXPECT_EQ(1, c3.get());

        // Now we destroy the reaper, *before* the pass at t == 5.0.
        // Because CallbackOnDestroy is set, e1 is reaped.
        r->destroy();
        EXPECT_EQ(0, r->size());
        EXPECT_EQ(1, c1.get());    // Callback for e1 must have been invoked.
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(1, c3.get());
    }

    {
        auto r = Reaper::create(1, 2, Reaper::CallbackOnDestroy);

        // Add three entries. This wakes up the reaper thread,
        // which schedules the next reaping pass 2 seconds
        // from now. The callback for e1 delays the reaping pass for a second,
        // so e2 and e3 will be looked at 1 second after reaping e1.

        // t == 0
        Counter c1(1000);
        auto e1 = r->add(bind(&Counter::increment, &c1));
        Counter c2;
        auto e2 = r->add(bind(&Counter::increment, &c2));
        Counter c3;
        auto e3 = r->add(bind(&Counter::increment, &c3));
        EXPECT_EQ(3, r->size());

        this_thread::sleep_for(chrono::milliseconds(2500));  // At t == 2.0, reaper kicks in.

        // t == 2.5
        EXPECT_EQ(1, c1.get());             // Was reaped at t == 2.0
        EXPECT_EQ(0, c2.get());             // Expires at t == 3.0, Due to be reaped at t == 4.0
        EXPECT_EQ(0, c3.get());             // Expires at t == 3.0, Due to be reaped at t == 4.0
        EXPECT_EQ(2, r->size());

        // t == 2.5
        // We destroy null out e2 and destroy e3 while the reaping pass is still stuck in the callback for e1.
        e2 = nullptr;
        e3->destroy();
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0, c3.get());

        this_thread::sleep_for(chrono::seconds(1));

        // t == 3.5
        // Callbacks for e2 and e3 are not invoked because they were destroyed during the reaping pass.
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0, c3.get());
        EXPECT_EQ(0, r->size());

        r->destroy();
        EXPECT_EQ(0, r->size());
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
        EXPECT_STREQ("unity::InvalidArgumentException: Reaper: invalid reap_interval (0). Interval must be > 0.",
                     e.what());
    }

    try
    {
        Reaper::create(2, 1);
        FAIL();
    }
    catch (unity::LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: Reaper: reap_interval (2) must be <= expiry_interval (1).",
                     e.what());
    }

    try
    {
        auto r = Reaper::create(1, 2);
        r->add(std::function<void()>());
        FAIL();
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: Reaper: invalid null callback passed to add().",
                     e.what());
    }

    try
    {
        Counter c;
        auto r = Reaper::create(1, 2);
        r->destroy();
        r->destroy();  // OK, no-op
        r->add(bind(&Counter::increment, &c));
        FAIL();
    }
    catch (unity::LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: Reaper: cannot add item to destroyed reaper.",
                     e.what());
    }

    // Make sure that if a callback throws an exception, this does no harm.
    Counter c;
    ReapItem::SPtr ri;
    {
        auto r = Reaper::create(5, 5, Reaper::CallbackOnDestroy);
        ri = r->add(bind(&Counter::increment_throw, &c));
        EXPECT_EQ(1u, r->size());
        EXPECT_EQ(0, c.get());
    }
    EXPECT_EQ(1, c.get());
}
