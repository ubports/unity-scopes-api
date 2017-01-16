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

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/MWObjectProxy.h>
#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <unity/scopes/ScopeExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

string const runtime_ini = TEST_DIR "/Runtime.ini";
string const zmq_ini = TEST_DIR "/Zmq.ini";

// Basic test.

TEST(ZmqMiddleware, basic)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);
    mw.start();
    sleep(1);
    mw.stop();
}

TEST(ZmqMiddleware, string_to_proxy)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    ObjectProxy p;
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
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

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

static constexpr int num_threads = 10;
int num_waiters;
int num_returned;
mutex waiter_mutex;
condition_variable waiter_cond;

void wait_for_shutdown_thread(ZmqMiddleware* mw)
{
    {
        lock_guard<mutex> lock(waiter_mutex);
        if (++num_waiters == num_threads)
        {
            waiter_cond.notify_all();
        }
    }
    mw->wait_for_shutdown();
    {
        // Allows us to test that threads are actually sleeping in wait_for_shutdown().
        lock_guard<mutex> lock(waiter_mutex);
        ++num_returned;
    }
}

// Make sure that multiple threads calling wait_for_shutdown() complete
// if the middleware was never started.

TEST(ZmqMiddleware, shutdown_before_start)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    vector<thread> threads;
    num_waiters = 0;
    num_returned = 0;
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(thread(wait_for_shutdown_thread, &mw));
    }
    {
        unique_lock<mutex> lock(waiter_mutex);
        waiter_cond.wait(lock, [] { return num_waiters == num_threads; });
    }
    for (int i = 0; i < num_threads; ++i)
    {
        threads[i].join();
    }
    mw.wait_for_shutdown();
}

// Make sure that multiple threads calling wait_for_shutdown() complete
// if the middleware was started.

TEST(ZmqMiddleware, shutdown_after_start)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    vector<thread> threads;
    num_waiters = 0;
    num_returned = 0;

    mw.start();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(thread(wait_for_shutdown_thread, &mw));
    }

    {
        // Wait for the threads to be ready. We sleep here for a moment
        // to give the threads a chance to actually call wait_for_shutdown().
        // (It's theoretially possible for num_threads == num_waiters before
        // *any* thread has called wait_for_shutdown().)
        this_thread::sleep_for(chrono::milliseconds(100));
        unique_lock<mutex> lock(waiter_mutex);
        waiter_cond.wait(lock, [] { return num_waiters == num_threads; });
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    {
        unique_lock<mutex> lock(waiter_mutex);
        EXPECT_EQ(0, num_returned);             // Make sure they are actually sleeping
    }

    mw.stop();

    for (int i = 0; i < num_threads; ++i)
    {
        threads[i].join();
    }
    mw.wait_for_shutdown();
}

class MyScopeObject : public ScopeObjectBase
{
public:
    MyScopeObject() {}

    virtual MWQueryCtrlProxy search(CannedQuery const&,
                                          SearchMetadata const&,
                                          VariantMap const&,
                                          MWReplyProxy const&,
                                          InvokeInfo const&) override
    {
        return nullptr;
    }

    virtual MWQueryCtrlProxy activate(Result const&,
                                      ActionMetadata const&,
                                      MWReplyProxy const&,
                                      InvokeInfo const&) override
    {
        return nullptr;
    }

    virtual MWQueryCtrlProxy perform_action(Result const&,
                                            ActionMetadata const&,
                                            std::string const&,
                                            std::string const&,
                                            MWReplyProxy const&,
                                            InvokeInfo const&) override
    {
        return nullptr;
    }

    virtual MWQueryCtrlProxy activate_result_action(Result const&,
                                                    ActionMetadata const&,
                                                    std::string const&,
                                                    MWReplyProxy const&,
                                                    InvokeInfo const&) override
    {
        return nullptr;
    }

    virtual MWQueryCtrlProxy preview(Result const&,
                                     ActionMetadata const&,
                                     MWReplyProxy const&,
                                     InvokeInfo const&) override
    {
        return nullptr;
    }

    virtual ChildScopeList child_scopes() const override
    {
        return ChildScopeList();
    }

    virtual bool set_child_scopes(ChildScopeList const&) override
    {
        return false;
    }

    virtual bool debug_mode() const override
    {
        return false;
    }
};

// Make sure that multiple threads calling wait_for_shutdown() complete
// if the middleware was started and has an object adapter.

TEST(ZmqMiddleware, shutdown_after_start_with_adapter)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    auto so = make_shared<MyScopeObject>();
    mw.add_scope_object("fred", so, 1000);
    mw.start();

    vector<thread> threads;
    num_waiters = 0;
    num_returned = 0;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(thread(wait_for_shutdown_thread, &mw));
    }

    {
        // Wait for the threads to be ready. We sleep here for a moment
        // to give the threads a chance to actually call wait_for_shutdown().
        // Otherwise, it's theoretially possible for num_threads == num_waiters
        // before *any* thread has called wait_for_shutdown().
        this_thread::sleep_for(chrono::milliseconds(100));
        unique_lock<mutex> lock(waiter_mutex);
        waiter_cond.wait(lock, [] { return num_waiters == num_threads; });
    }
    this_thread::sleep_for(chrono::milliseconds(100));
    {
        unique_lock<mutex> lock(waiter_mutex);
        EXPECT_EQ(0, num_returned);             // Make sure they are actually sleeping
    }

    mw.stop();

    for (int i = 0; i < num_threads; ++i)
    {
        threads[i].join();
    }
    mw.wait_for_shutdown();
}
