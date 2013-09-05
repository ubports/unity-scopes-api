/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public Lzmqnse version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public Lzmqnse for more details.
 *
 * You should have received a copy of the GNU General Public Lzmqnse
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/zmq_middleware/ObjectAdapter.h>

#include <unity/api/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/api/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

#include <future>
#include <thread>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;
using namespace unity::api::scopes::internal::zmq_middleware;

// We use this to sleep in between adapter creation and shutdown. That's
// necessary because zmq closes sockets asynchronously. Without the wait,
// because we are binding to the same endpoint each time, we can get
// an occastional "address in use" exception because the new socket
// tries to bind while the old one is still in the process of destroying itself.

void wait(int millisec = 20)
{
    this_thread::sleep_for(chrono::milliseconds(millisec));
}


// Basic test.

TEST(ObjectAdapter, basic)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/Zmq.config",
                     (RuntimeImpl*)0x1);

    // Instantiate and destroy oneway and twoway adapters with single and multiple threads.
    {
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 1, ObjectAdapter::Type::Twoway);
        EXPECT_EQ("testscope", a.name());
        EXPECT_EQ("ipc://testscope", a.endpoint());
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 1, ObjectAdapter::Type::Oneway);
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 5, ObjectAdapter::Type::Twoway);
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 10, ObjectAdapter::Type::Oneway);
    }

    // Same thing, but with activation.
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 1, ObjectAdapter::Type::Twoway);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 1, ObjectAdapter::Type::Oneway);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 5, ObjectAdapter::Type::Twoway);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 10, ObjectAdapter::Type::Oneway);
        a.activate();
    }

    // Again, with explicit deactivation and waiting.
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 1, ObjectAdapter::Type::Twoway);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 1, ObjectAdapter::Type::Oneway);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 5, ObjectAdapter::Type::Twoway);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 10, ObjectAdapter::Type::Oneway);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
}

TEST(ObjectAdapter, state_change)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/Zmq.config",
                     (RuntimeImpl*)0x1);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 2, ObjectAdapter::Type::Twoway);
        // We fire a bunch of activate requests in a loop, in the hope that we end up
        // getting coverage for the transition to the Active state.
        for (auto i = 0; i < 500; ++i)
        {
            a.activate();
        }
        a.shutdown();
        a.wait_for_shutdown();
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", 2, ObjectAdapter::Type::Oneway);
        // We fire a bunch of activate requests in a loop, in the hope that we end up
        // getting coverage for the transition to the Active state.
        for (auto i = 0; i < 500; ++i)
        {
            a.activate();
        }
        a.shutdown();
        a.wait_for_shutdown();
    }
}

TEST(ObjectAdapter, wait_for_shutdown)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/Zmq.config",
                     (RuntimeImpl*)0x1);

    // Start the adapter and call shutdown() from a different thread after a delay, and wait for the
    // shutdown to complete. We check that the shutdown happens after at least the delay that was specified,
    // so we can be sure that wait_for_shutdown() actually waits.
    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", 5, ObjectAdapter::Type::Twoway);
    a.activate();
    int delay_millisecs = 100;

    packaged_task<void()> task([&]{ wait(delay_millisecs); a.shutdown(); });
    auto const start_time = chrono::steady_clock::now();
    thread(move(task)).detach();

    a.wait_for_shutdown();
    auto const now = chrono::steady_clock::now();
    auto const delay = chrono::duration_cast<chrono::milliseconds>(now - start_time);
    EXPECT_TRUE(delay_millisecs <= delay.count());
}

struct MyObject : public AbstractObject
{
};

TEST(ObjectAdapter, add_remove_find)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/zmq_middleware/Zmq.config",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", 5, ObjectAdapter::Type::Twoway);

    try
    {
        a.add("", nullptr);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ObjectAdapter::add(): invalid empty id (adapter: testscope)",
                  e.to_string());
    }

    try
    {
        a.add("fred", nullptr);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: ObjectAdapter::add(): invalid nullptr object (adapter: testscope)",
                  e.to_string());
    }

    shared_ptr<MyObject> o(new MyObject);
    a.add("fred", o);
    auto f = a.find("fred");
    EXPECT_EQ(o, f);

    try
    {
        a.add("fred", o);
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_EQ("unity::api::scopes::MiddlewareException: ObjectAdapter::add(): cannot add id \"fred\":"
                  " id already used (adapter: testscope)",
                  e.to_string());
    }

    a.remove("fred");
    try
    {
        a.remove("fred");
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_EQ("unity::api::scopes::MiddlewareException: ObjectAdapter::remove(): cannot remove id \"fred\":"
                  " id not present (adapter: testscope)",
                  e.to_string());
    }

    try
    {
        auto f = a.find("fred");
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_EQ("unity::api::scopes::MiddlewareException: ObjectAdapter::find(): cannot find id \"fred\":"
                  " id not present (adapter: testscope)",
                  e.to_string());
    }
}
