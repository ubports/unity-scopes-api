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
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>

#include <unity/scopes/internal/MWObjectProxy.h>
#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <unity/scopes/ScopeExceptions.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

// Basic test.

TEST(ZmqMiddleware, basic)
{
    ZmqMiddleware mw("testscope",
                     TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ZmqMiddleware/Zmq.ini",
                     (RuntimeImpl*)0x1);
    mw.start();
    sleep(1);
    mw.stop();
}

TEST(ZmqMiddleware, string_to_proxy)
{
    ZmqMiddleware mw("testscope",
                     TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ZmqMiddleware/Zmq.ini",
                     (RuntimeImpl*)0x1);

    Proxy p;
    ScopeProxy sp;

    p = mw.string_to_proxy("nullproxy:");
    EXPECT_EQ(nullptr, p);

    p = mw.string_to_proxy("ipc://path#id");
    EXPECT_EQ("ipc://path", p->endpoint());
    EXPECT_EQ("id", p->identity());
    EXPECT_EQ(-1, p->timeout());
    sp = dynamic_pointer_cast<Scope>(p);
    ASSERT_NE(nullptr, sp);

    p = mw.string_to_proxy("ipc://path#id!t=-1");
    EXPECT_EQ("ipc://path", p->endpoint());
    EXPECT_EQ("id", p->identity());
    EXPECT_EQ(-1, p->timeout());

    p = mw.string_to_proxy("ipc://path#id!t=500");
    EXPECT_EQ("ipc://path", p->endpoint());
    EXPECT_EQ("id", p->identity());
    EXPECT_EQ(500, p->timeout());

    p = mw.string_to_proxy("ipc://path#id!t=500!c=Registry");
    EXPECT_EQ("ipc://path", p->endpoint());
    EXPECT_EQ("id", p->identity());
    EXPECT_EQ(500, p->timeout());

    p = mw.string_to_proxy("ipc://path#id!t=500!c=Scope!m=t");
    EXPECT_EQ("ipc://path", p->endpoint());
    EXPECT_EQ("id", p->identity());
    EXPECT_EQ(500, p->timeout());
}

TEST(ZmqMiddleware, string_to_proxy_ex)
{
    ZmqMiddleware mw("testscope",
                     TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ZmqMiddleware/Zmq.ini",
                     (RuntimeImpl*)0x1);

    try
    {
        mw.string_to_proxy("");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): proxy string cannot be empty", e.what());
    }

    try
    {
        mw.string_to_proxy("x");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy scheme prefix: "
                     "\"x\" (expected \"ipc://\")",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://abc");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: missing # separator: "
                     "ipc://abc",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://#abc");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: empty endpoint path: "
                     "ipc://#abc",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: empty identity: "
                     "ipc://path#",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#!");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: empty identity: "
                     "ipc://path#!",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: invalid empty "
                     "field specification: ipc://path#id!",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=1!");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: invalid empty "
                     "field specification: ipc://path#id!t=1!",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!!t=1!");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: invalid empty "
                     "field specification: ipc://path#id!!t=1!",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=1!!");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: invalid empty "
                     "field specification: ipc://path#id!t=1!!",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: bad field "
                     "specification (\"t\"): ipc://path#id!t",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!tt");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: bad field "
                     "specification (\"tt\"): ipc://path#id!tt",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!x=1");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: invalid field "
                     "identifier (\"x=1\"): ipc://path#id!x=1",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=1!t=1");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: duplicate "
                     "field specification (\"t=\"): ipc://path#id!t=1!t=1",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!m=");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: bad mode "
                     "(\"m=\"): ipc://path#id!m=",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: bad timeout "
                     "value (\"t=\"): ipc://path#id!t=",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=x");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: bad timeout "
                     "value (\"t=x\"): ipc://path#id!t=x",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=1x");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: bad timeout "
                     "value (\"t=1x\"): ipc://path#id!t=1x",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=-2");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: string_to_proxy(): invalid proxy: bad timeout "
                     "value (\"t=-2\"): ipc://path#id!t=-2",
                     e.what());
    }

    try
    {
        mw.string_to_proxy("ipc://path#id!t=500!c=Scope!m=o");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: make_typed_proxy(): cannot create oneway proxies",
                     e.what());
    }
}
