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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class Timer
{
public:
    Timer()
        : start_(chrono::system_clock::now())
    {
    }

    int operator()()
    {
        auto now = chrono::system_clock::now();
        auto ms_elapsed = chrono::duration_cast<chrono::milliseconds>(now - start_).count();
        return ms_elapsed;
    }

private:
    chrono::time_point<std::chrono::system_clock> start_;
};

class Counter
{
public:
    Counter(int delay = 0, function<void()> cb = nullptr)
        : c(0), d(chrono::milliseconds(delay)), cb_(cb)
    {
    }
    virtual ~Counter() = default;
    void reset() { c = 0; }
    int get() { return c; }
    virtual void increment() { if (cb_) cb_(); ++c; this_thread::sleep_for(d); }
    void increment_throw() { if (cb_) cb_(); ++c; throw 42; }

private:
    atomic_int c;
    chrono::milliseconds d;
    function<void()> cb_;
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
        v[1]->cancel();  // Does nothing
        v[1]->cancel();  // Safe to call more than once
        v[1]->refresh(); // Safe to call after cancel
    }

    {
        // Make sure that calling cancel on a ReapItem removes that item from the reaper.
        Counter c;
        vector<ReapItem::SPtr> v;
        {
            auto r = Reaper::create(5, 5);
            for (auto i = 0; i < 10; ++i)
            {
                v.push_back(r->add(bind(&Counter::increment, &c)));
            }
            EXPECT_EQ(10u, r->size());
            v[0]->cancel();
            v[4]->cancel();
            v[9]->cancel();
            EXPECT_EQ(7u, r->size());
            // We call cancel again, to make sure that it's safe to call it twice even though the first time
            // around, the cancel actually removed the item.
            v[0]->cancel();
            v[4]->cancel();
            v[9]->cancel();
            EXPECT_EQ(7u, r->size());
        }
        EXPECT_EQ(0, c.get());
    }

    {
        // Make sure that, after refreshing an item, it still can be cancelled, that is, that the ReapItem
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
            v[0]->cancel();
            EXPECT_EQ(2u, r->size());
            v[0]->cancel();            // no-op
            EXPECT_EQ(2u, r->size());
            v[1]->cancel();
            v[2]->cancel();
            EXPECT_EQ(0u, r->size());
        }
        EXPECT_EQ(0, c.get());
    }

    {
        // Make sure that cancelling a reap item after explicitly destroying
        // the reaper still works and correctly removes the item from the list.
        auto r = Reaper::create(5, 5);
        auto e1 = r->add([]{});
        auto e2 = r->add([]{});
        auto e3 = r->add([]{});
        EXPECT_EQ(3u, r->size());
        e2->cancel();
        EXPECT_EQ(2u, r->size());

        r->destroy();

        EXPECT_EQ(2u, r->size());
        e3->cancel();
        EXPECT_EQ(1u, r->size());
        e1->cancel();
        EXPECT_EQ(0u, r->size());
    }
}

TEST(Reaper, expiry)
{
    {
        Counter c;
        auto r = Reaper::create(1, 3);

        // Entries expire after 3 seconds.
        auto e1 = r->add(bind(&Counter::increment, &c));
        auto e2 = r->add(bind(&Counter::increment, &c));

        // Two seconds later, they still must both be there.
        this_thread::sleep_for(chrono::milliseconds(2000));
        EXPECT_EQ(2u, r->size());
        EXPECT_EQ(0, c.get());

        // Refresh one of the entries.
        e2->refresh();

        // 1.5 seconds later, one of them must have disappeared.
        this_thread::sleep_for(chrono::milliseconds(1500));
        EXPECT_EQ(1u, r->size());
        EXPECT_EQ(1, c.get());

        // 0.2 seconds later, the second entry must still be around.
        this_thread::sleep_for(chrono::milliseconds(200));
        EXPECT_EQ(1u, r->size());
        EXPECT_EQ(1, c.get());

        // 1.5 seconds later, the second entry must have disappeared.
        this_thread::sleep_for(chrono::milliseconds(1500));
        EXPECT_EQ(0u, r->size());
        EXPECT_EQ(2, c.get());
    }
}

TEST(Reaper, cancel_during_expiry)
{
    {
        Counter c1;
        Counter c2;
        Counter c3;

        auto r = Reaper::create(3, 3, Reaper::CallbackOnDestroy);

        // Add three entries. This wakes up the reaper thread,
        // which schedules the next reaping pass 3 seconds from now.
        // t == 0
        auto e1 = r->add(bind(&Counter::increment, &c1));
        auto e2 = r->add(bind(&Counter::increment, &c2));
        auto e3 = r->add(bind(&Counter::increment, &c3));

        // Wait 1.5 seconds and refresh e3.
        this_thread::sleep_for(chrono::milliseconds(1500));
        e3->refresh();

        // t == 1.5
        // Remaining life times:
        // e1: 1.5
        // e2: 1.5
        // e3: 3.0

        // Wait for 1 second and refresh e2.
        this_thread::sleep_for(chrono::seconds(1));
        e2->refresh();

        // t == 2.5
        // Remaining life times:
        // e1: 0.5
        // e2: 3.0
        // e3: 2.0
        //
        // Wait for 1.5 seconds. While we are asleep, the first
        // Reaping pass happens.
        this_thread::sleep_for(chrono::milliseconds(1500));  // At t == 3.0, first reaping pass, next pass at t == 6.0

        // t == 4.0
        // While we were asleep, the first pass must have reaped e1.
        EXPECT_EQ(2u, r->size());
        EXPECT_EQ(1, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0, c3.get());

        // t == 4.0
        // Remaining life times:
        // e2: 1.5
        // e3: 0.5

        // The first reaping pass happened 1 second ago, and the next
        // pass won't happen for another 2 seconds, because we reap
        // at most once every 3 seconds.
        // We sleep another second, which expires e3.
        this_thread::sleep_for(chrono::milliseconds(1000));

        // t== 5.0
        // Remaining life times:
        // e2: 0.5
        // e3: -0.5 (expired)
        EXPECT_EQ(2u, r->size());  // e3 is still there because it is expired, but not reaped.
        EXPECT_EQ(1, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0, c3.get());

        // We destroy e2, so its callback won't be called.
        e2 = nullptr;
        EXPECT_EQ(1u, r->size());  // e3 is still there because it is expired, but not reaped.
        EXPECT_EQ(1, c1.get());
        EXPECT_EQ(0, c2.get());    // Callback for e2 must not have been invoked.
        EXPECT_EQ(0, c3.get());

        // Now we destroy the reaper, *before* the pass at t == 6.0.
        // Because CallbackOnDestroy is set, e3 is reaped.
        r->destroy();
        EXPECT_EQ(0u, r->size());
        EXPECT_EQ(1, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(1, c3.get());    // Callback for e3 must have been invoked.
    }
}

TEST(Reaper, wait_during_expiry)
{
    {
        // Make sure that calls to cancel() wait if a reaping pass is
        // is in progress so the expiry callback is guaranteed to have
        // completed by the time cancel() returns.

        // t == 0
        Timer t;

        // Callback for c1 must arrive no sooner than 2 seconds from now, and no later
        // than 2.5 seconds from now.
        Counter c1(1000, [&t]{ int now = t(); if (now < 2000) FAIL(); if (now > 2500) FAIL(); });

        // Callback for c2 must arrive no sooner than 3 seconds from now.
        Counter c2(0, [&t] { if (t() < 3000) FAIL(); });

        auto r = Reaper::create(1, 2, Reaper::CallbackOnDestroy);

        // Add three entries. This wakes up the reaper thread,
        // which schedules the next reaping pass 2 seconds
        // from now. The callback for e1 delays the reaping pass for a second,
        // so e2 will be looked at 1 second after reaping e1.
        auto e1 = r->add(bind(&Counter::increment, &c1));
        auto e2 = r->add(bind(&Counter::increment, &c2));

        // e1 is oldest, so at the tail of the reap list.

        EXPECT_EQ(2u, r->size());
        EXPECT_LT(t(), 2000);

        this_thread::sleep_for(chrono::milliseconds(2500));  // At t == 2.0, reaper kicks in.

        // t == 2.5
        EXPECT_GE(t(), 2500);
        EXPECT_EQ(1, c1.get());             // Was reaped at t ~ 2.0 (tested by lambda on c1)
        EXPECT_EQ(0, c2.get());             // Expires at t ~ 2.0, but not reaped yet, because c1 has not completed
        EXPECT_EQ(1u, r->size());            // e2 must still be there

        // We null out e2 while the reaping pass is still stuck in the callback for e1.
        EXPECT_LT(t(), 3000);
        e2 = nullptr;                       // Blocks until e1's callback completes at t ~ 3.0 (tested by lambda on c2)
        EXPECT_GE(t(), 3000);
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0u, r->size());
    }
}

TEST(Reaper, self_cancel)
{
    {
        auto r = Reaper::create(1, 1, Reaper::CallbackOnDestroy);

        struct SelfDestroy
        {
            SelfDestroy(Reaper::SPtr const& r)
                : cb_called(false)
            {
                lock_guard<mutex> lock(m);
                ri = r->add([this]{ this->expired(); });
            }

            void expired()
            {
                lock_guard<mutex> lock(m);
                ri->cancel();
                cb_called = true;
            }

            bool called()
            {
                lock_guard<mutex> lock(m);
                return cb_called;
            }

        private:
            ReapItem::SPtr ri;
            bool cb_called;
            mutex m;
        };

        // Make sure that a callback that calls cancel() on its own
        // reap item doesn't deadlock.
        SelfDestroy sd(r);
        this_thread::sleep_for(chrono::milliseconds(1500));
        EXPECT_TRUE(sd.called());
        EXPECT_EQ(0u, r->size());
    }
}

TEST(Reaper, no_reap_thread)
{
    // Check that, with disabled reaper thread, we can still add,
    // refresh, and cancel entries, and that the callbacks for
    // entries that are still around at destruction time are
    // invoked (because CallbackOnDestroy is set).
    {
        Counter c1;
        Counter c2;
        Counter c3;

        auto r = Reaper::create(-1, -1, Reaper::CallbackOnDestroy);
        auto e1 = r->add(bind(&Counter::increment, &c1));
        auto e2 = r->add(bind(&Counter::increment, &c2));
        auto e3 = r->add(bind(&Counter::increment, &c3));

        e1->refresh();
        e2->cancel();
        EXPECT_EQ(2u, r->size());
        EXPECT_EQ(0, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0, c3.get());

        r->destroy();
        EXPECT_EQ(0u, r->size());
        EXPECT_EQ(1, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(1, c3.get());
    }

    // Same again, but with NoCallbackOnDestroy.
    {
        Counter c1;
        Counter c2;
        Counter c3;

        auto r = Reaper::create(-1, -1, Reaper::NoCallbackOnDestroy);
        auto e1 = r->add(bind(&Counter::increment, &c1));
        auto e2 = r->add(bind(&Counter::increment, &c2));
        auto e3 = r->add(bind(&Counter::increment, &c3));

        e1->refresh();
        e2->cancel();
        EXPECT_EQ(2u, r->size());
        EXPECT_EQ(0, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0, c3.get());

        r->destroy();
        EXPECT_EQ(2u, r->size());
        EXPECT_EQ(0, c1.get());
        EXPECT_EQ(0, c2.get());
        EXPECT_EQ(0, c3.get());
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
