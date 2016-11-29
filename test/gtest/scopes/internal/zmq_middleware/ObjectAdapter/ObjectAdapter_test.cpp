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
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/zmq_middleware/ServantBase.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReceiver.h>
#include <unity/scopes/internal/zmq_middleware/ZmqSender.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#include <capnp/serialize.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

string const runtime_ini = TEST_DIR "/Runtime.ini";
string const zmq_ini = TEST_DIR "/Zmq.ini";

// We use this to sleep in between adapter creation and shutdown. That's
// necessary because zmq closes sockets asynchronously. Without the wait,
// because we are binding to the same endpoint each time, we can get
// an occastional "address in use" exception because the new socket
// tries to bind while the old one is still in the process of destroying itself.

void wait(int millisec = 200)
{
    this_thread::sleep_for(chrono::milliseconds(millisec));
}

// Basic test.

TEST(ObjectAdapter, basic)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    // Instantiate and destroy oneway and twoway adapters with single and multiple threads.
    {
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
        EXPECT_EQ(&mw, a.mw());
        EXPECT_EQ("testscope", a.name());
        EXPECT_EQ("ipc://testscope", a.endpoint());
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 1);
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 10);
    }

    // Same thing, but with activation.
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 1);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);
        a.activate();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 10);
        a.activate();
    }

    // Again, with explicit deactivation and waiting.
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
        a.wait_for_shutdown(); // Second call benign, returns immediately
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 1);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 10);
        a.activate();
        a.shutdown();
        a.wait_for_shutdown();
    }
}

TEST(ObjectAdapter, state_change)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 2);
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
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 2);
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
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 2);

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
            EXPECT_STREQ("unity::scopes::MiddlewareException: activate(): Object adapter in Destroyed "
                         "state (adapter: testscope)",
                         e.what());
        }
    }

    {
        // Two adapters on the same endpoint. Second one must enter failed state.
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 2);
        a.activate();
        ObjectAdapter b(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 2);
        try
        {
            b.activate();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::run_workers(): stop thread "
                         "failure (adapter: testscope):\n"
                         "    unity::scopes::MiddlewareException: StopPublisher(): publisher thread "
                         "failed (endpoint: inproc://testscope-stopper):\n"
                         "        Address already in use",
                         e.what());
        }

        try
        {
            b.shutdown();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: shutdown() [state_ == Failed]: "
                         "Object adapter in Failed state (adapter: testscope)",
                         e.what());
        }
        try
        {
            b.wait_for_shutdown();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: wait_for_shutdown(): "
                         "Object adapter in Failed state (adapter: testscope)",
                         e.what());
        }
    }
}

TEST(ObjectAdapter, wait_for_shutdown)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    // Start the adapter and call shutdown() from a different thread after a delay, and wait for the
    // shutdown to complete. We check that the shutdown happens after at least the delay that was specified,
    // so we can be sure that wait_for_shutdown() actually waits.
    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);
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

// Mock servant that does nothing but return success.

class MyServant : public ServantBase
{
public:
    MyServant() :
        ServantBase(make_shared<MyDelegate>(), { { "success_op", bind(&MyServant::success_op, this,
                                                                      placeholders::_1,
                                                                      placeholders::_2,
                                                                      placeholders::_3) } })
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
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);

    try
    {
        a.add("", nullptr);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ObjectAdapter::add(): invalid empty id (adapter: testscope)",
                     e.what());
    }

    try
    {
        a.add("fred", nullptr);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ObjectAdapter::add(): invalid nullptr object (adapter: testscope)",
                     e.what());
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
        EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::add(): cannot add id \"fred\":"
                     " id already in use (adapter: testscope)",
                     e.what());
    }

    a.remove("fred");
    try
    {
        a.remove("fred");
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::remove(): cannot remove id \"fred\":"
                     " id not present (adapter: testscope)",
                     e.what());
    }

    EXPECT_EQ(nullptr, a.find("fred").get());
}

TEST(ObjectAdapter, dispatch_oneway_to_twoway)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::ONEWAY);    // No good for twoway adapter.
    request.setId("id");
    request.setCat("cat");
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
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::push);
    s.connect("ipc://testscope");
    ZmqSender sender(s);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::TWOWAY);    // No good for oneway adapter.
    request.setId("id");
    request.setId("cat");
    request.setOpName("operation_name");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    // We need to wait a little while, otherwise the adapter will have shut down before
    // it receives the oneway message. No real test here--this is for coverage.
    wait(100);
}

TEST(ObjectAdapter, dispatch_not_exist)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
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
    request.setCat("cat");
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
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
        a.activate();

        zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
        s.connect("ipc://testscope");
        ZmqSender sender(s);
        ZmqReceiver receiver(s);

        capnp::MallocMessageBuilder b;
        auto request = b.initRoot<capnproto::Request>();
        request.setMode(capnproto::RequestMode::TWOWAY);
        request.setId("id");
        request.setCat("cat");
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
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 1);
        a.activate();

        zmqpp::socket s(*mw.context(), zmqpp::socket_type::push);
        s.connect("ipc://testscope");
        ZmqSender sender(s);

        capnp::MallocMessageBuilder b;
        auto request = b.initRoot<capnproto::Request>();
        request.setMode(capnproto::RequestMode::ONEWAY);
        request.setId("id");
        request.setCat("cat");
        // Bad header: missing operation name

        auto segments = b.getSegmentsForOutput();
        sender.send(segments);

        wait();  // Give message time to get out before destroying the adapter

        // No test here, for coverage only.
    }
}

TEST(ObjectAdapter, corrupt_header)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
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
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 1);
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
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
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
    request.setCat("some_cat");
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
        ServantBase(make_shared<MyDelegate>(), { { "ONE_op", bind(&ThrowONEServant::ONE_op, this,
                                                                  placeholders::_1,
                                                                  placeholders::_2,
                                                                  placeholders::_3) } })
    {
    }

    virtual void ONE_op(Current const& current,
                        capnp::AnyPointer::Reader&,
                        capnproto::Response::Builder& r)
    {
        r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
        marshal_object_not_exist_exception(r, current);
    }
};

TEST(ObjectAdapter, invoke_object_not_exist)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
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
    request.setCat("some_cat");
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
    EXPECT_STREQ("some_cat", proxy.getCategory().cStr());
    EXPECT_TRUE(one.hasAdapter());
    EXPECT_STREQ("testscope", one.getAdapter().cStr());
}

TEST(ObjectAdapter, invoke_operation_not_exist)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
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
    request.setCat("some_cat");
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
    EXPECT_STREQ("some_cat", proxy.getCategory().cStr());
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
        ServantBase(make_shared<MyDelegate>(), { { "count_op", bind(&CountingServant::count_op,
                                                                    this,
                                                                    placeholders::_1,
                                                                    placeholders::_2,
                                                                    placeholders::_3) } }),
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

void invoke_thread(ZmqMiddleware* mw, RequestMode t, string const& object_id)
{
    zmqpp::socket s(*mw->context(), t == RequestMode::Twoway ? zmqpp::socket_type::request : zmqpp::socket_type::push);
    s.set(zmqpp::socket_option::linger, 200);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(t == RequestMode::Twoway ? capnproto::RequestMode::TWOWAY : capnproto::RequestMode::ONEWAY);
    request.setId(object_id);
    request.setCat("some_cat");
    request.setOpName("count_op");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    if (t == RequestMode::Twoway)
    {
        auto reply_segments = receiver.receive();
        capnp::SegmentArrayMessageReader reader(reply_segments);
        auto response = reader.getRoot<capnproto::Response>();
        EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::SUCCESS);
    }
}

TEST(ObjectAdapter, twoway_threading)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    // Single servant to which we send requests concurrently.
    shared_ptr<CountingServant> o(new CountingServant(100));

    const int num_threads = 5;
    const int num_requests = 20;
    {
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, num_threads);
        a.activate();

        a.add("some_id", o);

        // Send num_requests, each from its own thread.
        vector<thread> invokers;
        for (auto i = 0; i < num_requests; ++i)
        {
            invokers.push_back(thread(invoke_thread, &mw, RequestMode::Twoway, "some_id"));
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
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    // Single servant to which we send requests concurrently.
    shared_ptr<CountingServant> o(new CountingServant(100));

    const int num_threads = 5;
    const int num_requests = 20;
    {
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, num_threads);
        a.activate();

        a.add("some_id", o);

        // Send num_requests, each from its own thread.
        vector<thread> invokers;
        for (auto i = 0; i < num_requests; ++i)
        {
            invokers.push_back(thread(invoke_thread, &mw, RequestMode::Oneway, "some_id"));
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

// Show that a slow twoway invocation does not delay processing of other twoway invocations if
// the number of outstanding invocations exceeds the number of worker threads.

TEST(ObjectAdapter, load_balancing_twoway)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    // Twoway adapter with 3 threads.
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 3);
    a.activate();

    // Add servants that take 50 ms (fast) and 1000 ms (slow)
    shared_ptr<CountingServant> slow_servant(new CountingServant(1000));
    shared_ptr<CountingServant> fast_servant(new CountingServant(50));
    a.add("slow", slow_servant);
    a.add("fast", fast_servant);

    // Send a single request to the slow servant, and 30 requests to the fast servant.
    // The slow servant ties up a thread for a second, so the other two threads
    // should be processing the fast invocations during that time, meaning that the
    // 31 requests should complete in about a second.

    auto start_time = chrono::system_clock::now();

    vector<thread> invokers;
    invokers.push_back(thread(invoke_thread, &mw, RequestMode::Twoway, "slow"));
    for (auto i = 0; i < 30; ++i)
    {
        invokers.push_back(thread(invoke_thread, &mw, RequestMode::Twoway, "fast"));
    }
    for (auto& i : invokers)
    {
        i.join();
    }

    // We set a generous limit of two seconds, even though the entire thing will normally
    // finish in about 1.1 seconds, in case we are slow on Jenkins.
    auto end_time = chrono::system_clock::now();
    EXPECT_LT(chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count(), 2000);

    // We must have had 1 request on the slow servant, and 30 on the fast servant, with the
    // fast servant getting at least 2 invocations at the same time. Depending on scheduling
    // order, it's possible for a fast request to be sent before the single slow request,
    // we may have max concurrency of 3 in the fast servant occasionally.)
    EXPECT_EQ(1, slow_servant->num_invocations());
    EXPECT_EQ(30, fast_servant->num_invocations());
    EXPECT_GE(fast_servant->max_concurrent(), 2);
}

// Show that a slow oneway invocation does not delay processing of other oneway invocations if
// the number of outstanding invocations exceeds the number of worker threads.

TEST(ObjectAdapter, load_balancing_oneway)
{
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    // Oneway adapter with 3 threads.
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Oneway, 3);
    a.activate();

    // Add servants that take 10 ms (fast) and 1000 ms (slow)
    shared_ptr<CountingServant> slow_servant(new CountingServant(1000));
    shared_ptr<CountingServant> fast_servant(new CountingServant(10));
    a.add("slow", slow_servant);
    a.add("fast", fast_servant);

    // Socket to invoke on adapter
    zmqpp::socket s(*mw.context(), zmqpp::socket_type::push);
    s.set(zmqpp::socket_option::linger, 200);
    s.connect("ipc://testscope");
    ZmqSender sender(s);

    // Request for invoking slow servant.
    capnp::MallocMessageBuilder slow_b;
    auto slow_req = slow_b.initRoot<capnproto::Request>();
    slow_req.setMode(capnproto::RequestMode::ONEWAY);
    slow_req.setId("slow");
    slow_req.setCat("some_cat");
    slow_req.setOpName("count_op");

    // Request for invoking fast servant.
    capnp::MallocMessageBuilder fast_b;
    auto fast_req = fast_b.initRoot<capnproto::Request>();
    fast_req.setMode(capnproto::RequestMode::ONEWAY);
    fast_req.setId("fast");
    fast_req.setCat("some_cat");
    fast_req.setOpName("count_op");

    // Send a single request to the slow servant, and 140 requests to the fast servant.
    // The slow servant ties up a thread for a second, so the other two threads
    // are processing the fast invocations during that time, meaning that the
    // 141 requests should complete in about a second.
    auto slow_segments = slow_b.getSegmentsForOutput();
    sender.send(slow_segments);

    auto fast_segments = fast_b.getSegmentsForOutput();
    for (int i = 0; i < 140; ++i)
    {
        sender.send(fast_segments);
    }

    // Oneway invocations, so we need to give them a chance to finish.
    // We set a generous limit of two seconds, even though the entire thing will normally
    // finish in about 1.1 seconds, in case we are slow on Jenkins.
    this_thread::sleep_for(chrono::seconds(2));

    // We must have had 1 request on the slow servant, and 140 on the fast servant, with the
    // fast servant getting 2 invocations concurrently.
    EXPECT_EQ(1, slow_servant->num_invocations());
    EXPECT_EQ(140, fast_servant->num_invocations());
    EXPECT_EQ(2, fast_servant->max_concurrent());
}

using namespace std::placeholders;

// Servant that updates the servant map in various ways from its destructor, to verify
// that no deadlock can arise if a servant does this.

class UpdaterServant : public ServantBase
{
public:
    UpdaterServant(function<void()> func) :
        ServantBase(make_shared<MyDelegate>(), { { "op", bind(&UpdaterServant::op,
                                                              this,
                                                              placeholders::_1,
                                                              placeholders::_2,
                                                              placeholders::_3) } }),
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
    auto rt = RuntimeImpl::create("testscope", runtime_ini);
    ZmqMiddleware mw("testscope", rt.get(), zmq_ini);

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);

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
                EXPECT_STREQ("unity::scopes::MiddlewareException: remove(): "
                             "Object adapter in Destroyed state (adapter: testscope)",
                             e.what());
            }
        };
        a.add("fred", make_shared<UpdaterServant>(test_func));
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);

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
                EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::remove(): "
                             "cannot remove id \"fred\": id not present (adapter: testscope)",
                             e.what());
            }
        };
        auto servant = make_shared<UpdaterServant>(test_func);
        a.add("fred", servant);
        a.remove("fred");
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);

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
                EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::remove(): "
                             "cannot remove id \"fred\": id not present (adapter: testscope)",
                             e.what());
            }
        };
        auto servant = make_shared<UpdaterServant>(test_func);
        a.add("fred", servant);
        a.activate();
        a.remove("fred");
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);

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
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);

        // Destroy active adapter with a bunch of servants in it
        a.add("fred", make_shared<MyServant>());
        a.add("joe", make_shared<MyServant>());
        a.add("mary", make_shared<MyServant>());
        a.activate();
    }

    {
        wait();
        ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 5);

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
            EXPECT_STREQ("unity::scopes::MiddlewareException: add(): Object adapter in Destroyed "
                         "state (adapter: testscope)",
                         e.what());
        }
        try
        {
            a.find("joe");
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: find(): Object adapter in Destroyed "
                         "state (adapter: testscope)",
                         e.what());
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

        ZmqMiddleware mw("testscope", nullptr, zmq_ini);

        wait();
        try
        {
            ObjectAdapter a(mw, "testscope", endpoint, RequestMode::Twoway, 5);
            a.activate();
            a.shutdown();
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter: pump thread failure "
                         "(adapter: testscope):\n"
                         "    unity::scopes::MiddlewareException: safe_bind(): address in use: ipc://testscope",
                         e.what());
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

        ZmqMiddleware mw("testscope", nullptr, zmq_ini);
        wait();
        try
        {
            ObjectAdapter a(mw, "testscope", endpoint, RequestMode::Twoway, 5);
            a.activate();
            a.shutdown();
            FAIL();
        }
        catch (MiddlewareException const& e)
        {
            EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter: pump thread failure "
                         "(adapter: testscope):\n"
                         "    unity::scopes::MiddlewareException: safe_bind(): address in use: ipc://testscope",
                         e.what());
        }

        {
            lock_guard<mutex> lock(server_mutex);
            test_finished = true;
            server_done.notify_one();
        }
        t.join();
    }
}

TEST(ObjectAdapter, dflt_servant)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
    a.activate();

    zmqpp::socket s(*mw.context(), zmqpp::socket_type::request);
    s.connect("ipc://testscope");
    ZmqSender sender(s);
    ZmqReceiver receiver(s);

    shared_ptr<MyServant> o(new MyServant);
    a.add_dflt_servant("some_cat", o);
    try
    {
        a.add_dflt_servant("some_cat", o);
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::add_dflt_servant(): "
                     "cannot add category \"some_cat\": category already in use (adapter: testscope)",
                     e.what());
    }

    capnp::MallocMessageBuilder b;
    auto request = b.initRoot<capnproto::Request>();
    request.setMode(capnproto::RequestMode::TWOWAY);
    request.setId("some_id");
    request.setCat("some_cat");
    request.setOpName("success_op");

    auto segments = b.getSegmentsForOutput();
    sender.send(segments);
    segments = receiver.receive();

    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    EXPECT_EQ(response.getStatus(), capnproto::ResponseStatus::SUCCESS);

    a.remove_dflt_servant("some_cat");
    try
    {
        a.remove_dflt_servant("some_cat");
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: ObjectAdapter::remove_dflt_servant(): "
                     "cannot remove category \"some_cat\": category not present (adapter: testscope)",
                     e.what());
    }
}

TEST(ObjectAdapter, dflt_servant_exceptions)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    wait();
    ObjectAdapter a(mw, "testscope", "ipc://testscope", RequestMode::Twoway, 1);
    a.activate();

    try
    {
        a.add_dflt_servant("", nullptr);
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: ObjectAdapter::add_dflt_servant(): "
                     "invalid nullptr object (adapter: testscope)",
                     e.what());
    }

    ObjectAdapter b(mw, "testscope2", "ipc://testscope", RequestMode::Oneway, 2);
    EXPECT_THROW(b.activate(), MiddlewareException);

    try
    {
        shared_ptr<MyServant> o(new MyServant);
        b.add_dflt_servant("some_cat", o);
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: add_dflt_servant(): Object adapter in"
                     " Failed state (adapter: testscope2)\n"
                     "    Exception history:\n"
                     "        Exception #1:\n"
                     "            unity::scopes::MiddlewareException: ObjectAdapter: pump thread failure"
                     " (adapter: testscope2):\n"
                     "                unity::scopes::MiddlewareException: safe_bind(): address in use: ipc://testscope",
                     e.what());
    }

    try
    {
        shared_ptr<MyServant> o(new MyServant);
        b.remove_dflt_servant("some_cat");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: remove_dflt_servant(): Object adapter in"
                     " Failed state (adapter: testscope2)\n"
                     "    Exception history:\n"
                     "        Exception #1:\n"
                     "            unity::scopes::MiddlewareException: ObjectAdapter: pump thread failure"
                     " (adapter: testscope2):\n"
                     "                unity::scopes::MiddlewareException: safe_bind(): address in use: ipc://testscope",
                     e.what());
    }

    try
    {
        shared_ptr<MyServant> o(new MyServant);
        b.find_dflt_servant("some_cat");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: find_dflt_servant(): Object adapter in"
                     " Failed state (adapter: testscope2)\n"
                     "    Exception history:\n"
                     "        Exception #1:\n"
                     "            unity::scopes::MiddlewareException: ObjectAdapter: pump thread failure"
                     " (adapter: testscope2):\n"
                     "                unity::scopes::MiddlewareException: safe_bind(): address in use: ipc://testscope",
                     e.what());
    }
}
