/*
 * Copyright (C) 2014 Canonical Ltd
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
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/zmq_middleware/StopPublisher.h>
#include <unity/scopes/ScopeExceptions.h>
#include <zmqpp/poller.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal::zmq_middleware;

TEST(StopPublisher, basic)
{
    {
        zmqpp::context c;
        StopPublisher p(&c, "x");
        EXPECT_EQ("inproc://x", p.endpoint());
    }

    {
        zmqpp::context c;
        StopPublisher p(&c, "x");
        p.stop();                       // no-op if stop() is called several times.
        p.stop();
        EXPECT_EQ("inproc://x", p.endpoint());
    }
}

TEST(StopPublisher, exceptions)
{
    {
        zmqpp::context c;
        StopPublisher p(&c, "x");
        p.stop();
        try
        {
            p.subscribe();
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: StopPublisher::subscribe(): "
                         "cannot subscribe to stopped publisher (endpoint: inproc://x)",
                         e.what());
        }
    }

    {
        zmqpp::context c;
        StopPublisher p(&c, "x");
        try
        {
            StopPublisher q(&c, "x");   // Second publisher at the same endpoint must fail
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: StopPublisher(): publisher thread "
                         "failed (endpoint: inproc://x):\n"
                         "    Address already in use",
                         e.what());
        }
    }
}

TEST(StopPublisher, subscribe)
{
    {
        zmqpp::context c;
        StopPublisher p(&c, "x");
        auto socket = p.subscribe();
        zmqpp::poller poller;
        poller.add(socket);
        poller.poll(100);
        EXPECT_FALSE(poller.has_input(socket));
        p.stop();
        poller.poll(100);
        EXPECT_TRUE(poller.has_input(socket));
    }
}

void subscriber_thread(StopPublisher* p)
{
    auto socket = p->subscribe();
    zmqpp::poller poller;
    poller.add(socket);
    poller.poll();          // Blocks indefinitely until socket is ready
}

void publisher_thread(unique_ptr<StopPublisher>)
{
    // Sleep for a while, so all the subscribers get a chance to block in poll().
    this_thread::sleep_for(chrono::milliseconds(500));
    // Destructor of unique_ptr in-param calls stop()
}

// Test that subscriber threads terminate correctly even if the destructor of the
// publisher is called from a different thread.

TEST(StopPublisher, threading)
{
    zmqpp::context c;
    unique_ptr<StopPublisher> p(new StopPublisher(&c, "x"));
    const int num_subscribers = 5;
    vector<thread> subscribers;
    for (int i = 0; i < num_subscribers; ++i)
    {
        subscribers.emplace_back(thread(subscriber_thread, p.get()));
    }

    thread publisher(publisher_thread, move(p));  // Moves publisher to publisher thread

    for (auto& s : subscribers)
    {
        s.join();
    }

    publisher.join();
}
