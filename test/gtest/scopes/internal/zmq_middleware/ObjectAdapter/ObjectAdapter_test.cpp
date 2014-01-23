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

#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>

#include <scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/scopes/internal/zmq_middleware/ServantBase.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReceiver.h>
#include <unity/scopes/internal/zmq_middleware/ZmqSender.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#include <capnp/serialize.h>
#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

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
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    // Instantiate and destroy oneway and twoway adapters with single and multiple threads.
    {
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
        EXPECT_EQ(&mw, a.mw());
        EXPECT_EQ("testscope", a.name());
        EXPECT_EQ("ipc://testscope", a.endpoint());
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 1);
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 10);
    }

    // Same thing, but with activation.
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 1);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 10);
        a.activate();
    }

    // Again, with explicit deactivation and waiting.
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
        a.wait_for_shutdown(); // Second call benign, returns immediately
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 1);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 10);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
}

TEST(ObjectAdapter, state_change)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 2);
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
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 2);
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
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 2);

        // Try to reactivate after shutdown
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
        try
        {
            a.activate();
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_EQ("unity::scopes::MiddlewareException: Object adapter in Destroyed "
                      "state (adapter: testscope)",
                      e.to_string());
        }
    }

    {
        // Two adapters on the same endpoint. Second one must enter failed state.
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 2);
        a.activate();
        ObjectAdapter b(mw, "testscope", "ipc://testscope", RequestType::Oneway, 2);
        try
        {
            b.activate();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_EQ("unity::scopes::MiddlewareException: ObjectAdapter::run_workers(): broker thread "
                      "failure (adapter: testscope):\n"
                      "    unity::scopes::MiddlewareException: ObjectAdapter: broker thread failure "
                      "(adapter: testscope):\n"
                      "        Address already in use",
                      e.to_string());
        }
        try
        {
            b.shutdown();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_EQ("unity::scopes::MiddlewareException: Object adapter in Failed state (adapter: testscope)\n"
                      "    Exception history:\n"
                      "        Exception #1:\n"
                      "            unity::scopes::MiddlewareException: ObjectAdapter: broker thread failure "
                      "(adapter: testscope):\n"
                      "                Address already in use",
                      e.to_string());
        }
        try
        {
            b.wait_for_shutdown();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_EQ("unity::scopes::MiddlewareException: Object adapter in Failed state (adapter: testscope)\n"
                       "    Exception history:\n"
                       "        Exception #1:\n"
                       "            unity::scopes::MiddlewareException: ObjectAdapter: broker thread failure "
                       "(adapter: testscope):\n"
                       "                Address already in use",
                       e.to_string());
        }
    }
}

TEST(ObjectAdapter, wait_for_shutdown)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    // Start the adapter and call shutdown() from a different thread after a delay, and wait for the
    // shutdown to complete. We check that the shutdown happens after at least the delay that was specified,
    // so we can be sure that wait_for_shutdown() actually waits.
    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);
    a.activate();
    int delay_millisecs = 100;

    packaged_task<void()> task([&] { wait(delay_millisecs); a.shutdown(); });
    auto const start_time = chrono::steady_clock::now();
    thread(move(task)).detach();

    a.wait_for_shutdown();
    auto const now = chrono::steady_clock::now();
    auto const delay = chrono::duration_cast<chrono::milliseconds>(now - start_time);
    EXPECT_TRUE(delay_millisecs <= delay.count());
}

class MyDelegate : public AbstractObject
{
};

using namespace std::placeholders;

// Mock servant that does nothing but return success.

class MyServant : public ServantBase
{
public:
    MyServant() :
        ServantBase(make_shared<MyDelegate>(), { { "success_op", bind(&MyServant::success_op, this, _1, _2, _3) } })
    {
    }

    virtual void success_op(Current const&,
                            capnp::AnyPointer::Reader&,
                            capnproto::Response::Builder& r)
    {
        r.setStatus(capnproto::ResponseStatus::SUCCESS);
    }
};

TEST(ObjectAdapter, add_remove_find)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);

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

    shared_ptr<MyServant> o(new MyServant);
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
        EXPECT_EQ("unity::scopes::MiddlewareException: ObjectAdapter::add(): cannot add id \"fred\":"
                  " id already in use (adapter: testscope)",
                  e.to_string());
    }

    a.remove("fred");
    try
    {
        a.remove("fred");
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_EQ("unity::scopes::MiddlewareException: ObjectAdapter::remove(): cannot remove id \"fred\":"
                  " id not present (adapter: testscope)",
                  e.to_string());
    }

    EXPECT_EQ(nullptr, a.find("fred").get());
}

TEST(ObjectAdapter, dispatch_oneway_to_twoway)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::ONEWAY);    // No good for twoway adapter.
    request.setId("id");
    request.setOpName("operation_name");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    segments = receiver.receive();

    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::RUNTIME_EXCEPTION);

    auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
    EXPECT_EQ(capnproto::RuntimeException::UNKNOWN, ex.which());
    EXPECT_STREQ("ObjectAdapter: oneway invocation sent to twoway adapter"
                 " (id: id, adapter: testscope, op: operation_name)",
                 ex.getUnknown().cStr());
}

TEST(ObjectAdapter, dispatch_twoway_to_oneway)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::push);
    s.connect("ipc://testscope");
    ZmqSender sender(s);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::TWOWAY);    // No good for oneway adapter.
    request.setId("id");
    request.setOpName("operation_name");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    // We need to wait a little while, otherwise the adapter will have shut down before
    // it receives the oneway message. No real test here--this is for coverage.
    wait(100);
}

TEST(ObjectAdapter, dispatch_not_exist)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    a.activate();

    // No servant registered, check that we get an ObjectNotExistException
    zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::TWOWAY);
    request.setId("id");
    request.setOpName("operation_name");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    segments = receiver.receive();

    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::RUNTIME_EXCEPTION);

    auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
    EXPECT_EQ(capnproto::RuntimeException::OBJECT_NOT_EXIST, ex.which());

    auto one = ex.getObjectNotExist();
    EXPECT_TRUE(one.hasProxy());
    auto proxy = one.getProxy();
    EXPECT_EQ(a.endpoint(), proxy.getEndpoint().cStr());
    EXPECT_STREQ("id", proxy.getIdentity().cStr());
    EXPECT_TRUE(one.hasAdapter());
    EXPECT_STREQ("testscope", one.getAdapter().cStr());
}

TEST(ObjectAdapter, bad_header)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
        a.activate();

        zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
        s.connect("ipc://testscope");
        ZmqSender sender(s);
        ZmqReceiver receiver(s);

        capnp::MallocMessageBuilder b;
        auto request = b.initRoot<capnproto::Request>();
        request.setMode(capnproto::RequestMode::TWOWAY);
        request.setId("id");
        // Bad header: missing operation name

        auto segments = b.getSegmentsForOutput();
        sender.send(segments);
        segments = receiver.receive();

        capnp::SegmentArrayMessageReader reader(segments);
        auto response = reader.getRoot<capnproto::Response>();
        EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::RUNTIME_EXCEPTION);

        auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
        EXPECT_EQ(capnproto::RuntimeException::UNKNOWN, ex.which());
        EXPECT_STREQ("Invalid message header", ex.getUnknown().cStr());
    }

    // Invalid message header a second time, with oneway adapter (for coverage)
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 1);
        a.activate();

        zmqpp::socket s(*mw.context(), zmqpp::socket_type::push);
        s.connect("ipc://testscope");
        ZmqSender sender(s);

        capnp::MallocMessageBuilder b;
        auto request = b.initRoot<capnproto::Request>();
        request.setMode(capnproto::RequestMode::ONEWAY);
        request.setId("id");
        // Bad header: missing operation name

        auto segments = b.getSegmentsForOutput();
        sender.send(segments);

        wait();  // Give message time to get out before destroying the adapter

        // No test here, for coverage only.
    }
}

TEST(ObjectAdapter, corrupt_header)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
        a.activate();

        zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
        s.connect("ipc://testscope");
        ZmqSender sender(s);
        ZmqReceiver receiver(s);

        // Make a malformed message header so we get coverage on the exception case.
        capnp::word buf[1];
        *reinterpret_cast<uint64_t*>(buf) = 0x99;
        kj::ArrayPtr<capnp::word const> badword(&buf[0], 1);
        kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> segments(&badword, 1);
        sender.send(segments);

        segments = receiver.receive();

        capnp::SegmentArrayMessageReader reader(segments);
        auto response = reader.getRoot<capnproto::Response>();
        EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::RUNTIME_EXCEPTION);

        auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
        EXPECT_EQ(capnproto::RuntimeException::UNKNOWN, ex.which());
        string msg = ex.getUnknown().cStr();
        boost::regex r("ObjectAdapter: error unmarshaling request header.*");
        EXPECT_TRUE(boost::regex_match(msg, r));
    }

    // Malformed message header a second time, with oneway adapter (for coverage)
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, 1);
        a.activate();

        zmqpp::socket s(*mw.context(), zmqpp::socket_type::push);
        s.connect("ipc://testscope");
        ZmqSender sender(s);
        ZmqReceiver receiver(s);

        // Make a malformed message header so we get coverage on the exception case.
        capnp::word buf[1];
        *reinterpret_cast<uint64_t*>(buf) = 0x99;
        kj::ArrayPtr<capnp::word const> badword(&buf[0], 1);
        kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> segments(&badword, 1);
        sender.send(segments);

        wait();  // Give message time to get out before destroying the adapter

        // No test here, for coverage only.
    }
}

TEST(ObjectAdapter, invoke_ok)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    shared_ptr<MyServant> o(new MyServant);
    a.add("some_id", o);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::TWOWAY);
    request.setId("some_id");
    request.setOpName("success_op");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    segments = receiver.receive();

    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::SUCCESS);
}

// Servant that returns object not exist

class ThrowONEServant : public ServantBase
{
public:
    ThrowONEServant() :
        ServantBase(make_shared<MyDelegate>(), { { "ONE_op", bind(&ThrowONEServant::ONE_op, this, _1, _2, _3) } })
    {
    }

    virtual void ONE_op(Current const& current,
                        capnp::AnyPointer::Reader&,
                        capnproto::Response::Builder& r)
    {
        r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
        marshal_object_not_exist_exception(r, current.id, current.adapter->endpoint(), current.adapter->name());
    }
};

TEST(ObjectAdapter, invoke_object_not_exist)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    shared_ptr<ThrowONEServant> o(new ThrowONEServant);
    a.add("some_id", o);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::TWOWAY);
    request.setId("some_id");
    request.setOpName("ONE_op");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    segments = receiver.receive();

    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::RUNTIME_EXCEPTION);

    auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
    EXPECT_EQ(capnproto::RuntimeException::OBJECT_NOT_EXIST, ex.which());

    auto one = ex.getObjectNotExist();
    EXPECT_TRUE(one.hasProxy());
    auto proxy = one.getProxy();
    EXPECT_EQ(a.endpoint(), proxy.getEndpoint().cStr());
    EXPECT_STREQ("some_id", proxy.getIdentity().cStr());
    EXPECT_TRUE(one.hasAdapter());
    EXPECT_STREQ("testscope", one.getAdapter().cStr());
}

TEST(ObjectAdapter, invoke_operation_not_exist)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    shared_ptr<MyServant> o(new MyServant);
    a.add("some_id", o);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::TWOWAY);
    request.setId("some_id");
    request.setOpName("operation_name");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    segments = receiver.receive();

    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::RUNTIME_EXCEPTION);

    auto ex = response.getPayload().getAs<capnproto::RuntimeException>();
    EXPECT_EQ(capnproto::RuntimeException::OPERATION_NOT_EXIST, ex.which());

    auto opne = ex.getOperationNotExist();
    EXPECT_TRUE(opne.hasProxy());
    auto proxy = opne.getProxy();
    EXPECT_EQ(a.endpoint(), proxy.getEndpoint().cStr());
    EXPECT_STREQ("some_id", proxy.getIdentity().cStr());
    EXPECT_TRUE(opne.hasAdapter());
    EXPECT_STREQ("testscope", opne.getAdapter().cStr());
    EXPECT_TRUE(opne.hasAdapter());
    EXPECT_STREQ("operation_name", opne.getOpName().cStr());
}

// Make sure that we do actually run threaded if the adapter has more than one thread.

class CountingServant : public ServantBase
{
public:
    CountingServant(int delay_millisecs) :
        ServantBase(make_shared<MyDelegate>(), { { "count_op", bind(&CountingServant::count_op, this, _1, _2, _3) } }),
        concurrent_(0),
        max_concurrent_(0),
        num_invocations_(0),
        delay_(delay_millisecs)
    {
    }

    virtual void count_op(Current const&,
                          capnp::AnyPointer::Reader&,
                          capnproto::Response::Builder& r)
    {
        ++num_invocations_;
        atomic_int num(++concurrent_);
        max_concurrent_.store(max(num, max_concurrent_));
        wait(delay_);
        --concurrent_;

        r.setStatus(capnproto::ResponseStatus::SUCCESS);
    }

    int max_concurrent() const noexcept
    {
        return max_concurrent_;
    }

    int num_invocations() const noexcept
    {
        return num_invocations_;
    }

private:
    atomic_int concurrent_;
    atomic_int max_concurrent_;
    atomic_int num_invocations_;
    int delay_;
};

void invoke_thread(ZmqMiddleware* mw, RequestType t)
{
    zmqpp::socket s(*mw->context(), t == RequestType::Twoway ? zmqpp::socket_type::request : zmqpp::socket_type::push);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(t == RequestType::Twoway ? capnproto::RequestMode::TWOWAY : capnproto::RequestMode::ONEWAY);
    request.setId("some_id");
    request.setOpName("count_op");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    if (t == RequestType::Twoway)
    {
        auto reply_segments = receiver.receive();
        capnp::SegmentArrayMessageReader reader(reply_segments);
        auto response = reader.getRoot<capnproto::Response>();
        EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::SUCCESS);
    }
    else
    {
        wait(50);  // Allow some time for oneway requests to actually make it on the wire.
    }
}

TEST(ObjectAdapter, twoway_threading)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    // Single servant to which we send requests concurrently.
    shared_ptr<CountingServant> o(new CountingServant(100));

    const int num_threads = 5;
    const int num_requests = 20;
    {
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, num_threads);
        a.activate();

        a.add("some_id", o);

        // Send num_requests, each from its own thread.
        vector<thread> invokers;
        for (auto i = 0; i < num_requests; ++i)
        {
            invokers.push_back(thread(invoke_thread, &mw, RequestType::Twoway));
        }
        for (auto& i : invokers)
        {
            i.join();
        }
    }

    // We must have had num_requests in total, at most num_threads of which were processed concurrently. The delay
    // in the servant ensures that we actually reach the maximum of num_threads concurrent invocations.
    EXPECT_EQ(num_requests, o->num_invocations());
    EXPECT_EQ(num_threads, o->max_concurrent());
}

TEST(ObjectAdapter, oneway_threading)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    // Single servant to which we send requests concurrently.
    shared_ptr<CountingServant> o(new CountingServant(100));

    const int num_threads = 5;
    const int num_requests = 20;
    {
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Oneway, num_threads);
        a.activate();

        a.add("some_id", o);

        // Send num_requests, each from its own thread.
        vector<thread> invokers;
        for (auto i = 0; i < num_requests; ++i)
        {
            invokers.push_back(thread(invoke_thread, &mw, RequestType::Oneway));
        }
        for (auto& i : invokers)
        {
            i.join();
        }

        // We need to delay here, otherwise we end up destroying the adapter before
        // the oneway invocations are processed. We process num_threads requests
        // in parallel, so the total time will be roughly the number of requests
        // divided by the number of threads (which is the number of "batches"),
        // plus a bit of slack.
        wait(((num_requests / num_threads) + 1) * 100 + 100);
    }

    // We must have had num_requests in total, at most num_threads of which were processed concurrently. The delay
    // in the servant ensures that we actually reach the maximum of num_threads concurrent invocations.

    EXPECT_EQ(num_requests, o->num_invocations());
    EXPECT_EQ(num_threads, o->max_concurrent());
}

using namespace std::placeholders;

// Servant that updates the servant map in various ways from its destructor, to verify
// that no deadlock can arise if a servant does this.

class UpdaterServant : public ServantBase
{
public:
    UpdaterServant(function<void()> func) :
        ServantBase(make_shared<MyDelegate>(), { { "op", bind(&UpdaterServant::op, this, _1, _2, _3) } }),
        func(func)
    {
    }

    ~UpdaterServant()
    {
        func();
    }

    virtual void op(Current const&,
                    capnp::AnyPointer::Reader&,
                    capnproto::Response::Builder& r)
    {
        r.setStatus(capnproto::ResponseStatus::SUCCESS);
    }

    function<void()> func;
};

TEST(ObjectAdapter, servant_map_destructor)
{
    ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                     (RuntimeImpl*)0x1);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);

        // Servant calls remove on itself from its destructor after adapter is destroyed
        auto test_func = [&]()
        {
            try
            {
                a.remove("fred");
                FAIL();
            }
            catch (MiddlewareException const& e)
            {
                EXPECT_EQ("unity::scopes::MiddlewareException: Object adapter in Destroyed state (adapter: testscope)",
                          e.to_string());
            }
        };
        a.add("fred", make_shared<UpdaterServant>(test_func));
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);

        // Servant calls remove on itself from its destructor while adapter is inactive
        auto test_func = [&]()
        {
            try
            {
                a.remove("fred");
                FAIL();
            }
            catch (MiddlewareException const& e)
            {
                EXPECT_EQ("unity::scopes::MiddlewareException: ObjectAdapter::remove(): "
                          "cannot remove id \"fred\": id not present (adapter: testscope)",
                          e.to_string());
            }
        };
        auto servant = make_shared<UpdaterServant>(test_func);
        a.add("fred", servant);
        a.remove("fred");
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);

        // Servant calls remove on itself from its destructor while adapter is active
        auto test_func = [&]()
        {
            try
            {
                a.remove("fred");
                FAIL();
            }
            catch (MiddlewareException const& e)
            {
                EXPECT_EQ("unity::scopes::MiddlewareException: ObjectAdapter::remove(): "
                          "cannot remove id \"fred\": id not present (adapter: testscope)",
                          e.to_string());
            }
        };
        auto servant = make_shared<UpdaterServant>(test_func);
        a.add("fred", servant);
        a.activate();
        a.remove("fred");
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);

        // Servant removes another servant from its destructor while adapter is active
        auto test_func = [&]()
        {
            EXPECT_NO_THROW(a.remove("joe"));
        };
        a.add("fred", make_shared<UpdaterServant>(test_func));
        a.add("joe", make_shared<MyServant>());
        a.activate();
        a.remove("fred");
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);

        // Destroy active adapter with a bunch of servants in it
        a.add("fred", make_shared<MyServant>());
        a.add("joe", make_shared<MyServant>());
        a.add("mary", make_shared<MyServant>());
        a.activate();
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestType::Twoway, 5);

        // Try adding and finding servant after adapter is destroyed
        a.add("fred", make_shared<MyServant>());
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
        try
        {
            a.add("joe", make_shared<MyServant>());
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_EQ("unity::scopes::MiddlewareException: Object adapter in Destroyed "
                      "state (adapter: testscope)",
                      e.to_string());
        }
        try
        {
            a.find("joe");
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_EQ("unity::scopes::MiddlewareException: Object adapter in Destroyed "
                      "state (adapter: testscope)",
                      e.to_string());
        }
    }
}

namespace
{

bool test_finished;
std::mutex server_mutex;
std::condition_variable server_done;

}

void
mock_server(string const& endpoint, zmqpp::socket_type stype)
{
    // Simulate a server bound to the endpoint already
    zmqpp::context c;
    zmqpp::socket s(c, stype);
    s.set(zmqpp::socket_option::linger, 0);
    s.set(zmqpp::socket_option::receive_timeout, 500);
    s.bind(endpoint);

    // Wait for test to tell us to go away
    unique_lock<mutex> lock(server_mutex);
    server_done.wait(lock, []{ return test_finished; });
}

TEST(ObjectAdapter, double_bind)
{
    const string endpoint = "ipc://testscope";

    {
        // Simulate a server bound to the endpoint already
        {
            lock_guard<mutex> lock(server_mutex);
            test_finished = false;
        }
        std::thread t(&mock_server, endpoint, zmqpp::socket_type::router);

        wait(200);  // Give zmq some time to finish the bind.

        ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                         (RuntimeImpl*)0x1);
        wait();
        try
        {
            ObjectAdapter a(mw, "testscope", endpoint, RequestType::Twoway, 5);
            a.activate();
            a.shutdown();
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::run_workers(): broker thread failure "
                         "(adapter: testscope):\n"
                         "    unity::scopes::MiddlewareException: ObjectAdapter: broker thread failure "
                         "(adapter: testscope): address in use: ipc://testscope", e.what());
        }

        {
            lock_guard<mutex> lock(server_mutex);
            test_finished = true;
            server_done.notify_one();
        }
        t.join();
    }

    // Same test again, but for pull socket
    {
        // Simulate a server bound to the endpoint already
        {
            lock_guard<mutex> lock(server_mutex);
            test_finished = false;
        }
        std::thread t(&mock_server, endpoint, zmqpp::socket_type::pull);

        wait(200);  // Give zmq some time to finish the bind.

        ZmqMiddleware mw("testscope", TEST_BUILD_ROOT "/gtest/scopes/internal/zmq_middleware/ObjectAdapter/Zmq.ini",
                         (RuntimeImpl*)0x1);
        wait();
        try
        {
            ObjectAdapter a(mw, "testscope", endpoint, RequestType::Twoway, 5);
            a.activate();
            a.shutdown();
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::run_workers(): broker thread failure "
                         "(adapter: testscope):\n"
                         "    unity::scopes::MiddlewareException: ObjectAdapter: broker thread failure "
                         "(adapter: testscope): address in use: ipc://testscope", e.what());
        }

        {
            lock_guard<mutex> lock(server_mutex);
            test_finished = true;
            server_done.notify_one();
        }
        t.join();
    }
}
