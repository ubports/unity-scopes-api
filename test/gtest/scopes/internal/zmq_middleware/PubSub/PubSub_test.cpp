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

#include <scope-api-testconfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/ScopeExceptions.h>

#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

TEST(PubSub, endpoints)
{
    ZmqMiddleware mw("testscope", (RuntimeImpl*)0x1,
                     TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini");

    auto publisher = mw.create_publisher("testpublisher", "");
    auto subscriber = mw.create_subscriber("testpublisher", "");

    EXPECT_EQ("ipc:///tmp/testpublisher-p", publisher->endpoint());
    EXPECT_EQ(subscriber->endpoint(), publisher->endpoint());
}

TEST(PubSub, exceptions)
{
    ZmqMiddleware mw("testscope", (RuntimeImpl*)0x1,
                     TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini");

    // Test that only one publisher can be created on a single endpoint
    auto publisher = mw.create_publisher("testpublisher", "");
    try
    {
        auto publisher2 = mw.create_publisher("testpublisher", "");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: ZmqPublisher(): publisher thread failed to start "
                     "(endpoint: ipc:///tmp/testpublisher-p):\n    unity::scopes::MiddlewareException: "
                     "ZmqPublisher::safe_bind(): endpoint in use: ipc:///tmp/testpublisher-p",
                     e.what());
    }

    // Test that multiple subscribers can be created on a single endpoint
    auto subscriber = mw.create_subscriber("testpublisher", "");
    try
    {
        auto subscriber2 = mw.create_subscriber("testpublisher", "");
        auto subscriber3 = mw.create_subscriber("testpublisher", "");
        auto subscriber4 = mw.create_subscriber("testpublisher", "");
        auto subscriber5 = mw.create_subscriber("testpublisher", "");
    }
    catch (MiddlewareException const& e)
    {
        FAIL();
    }
}
