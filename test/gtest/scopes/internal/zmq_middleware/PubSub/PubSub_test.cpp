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

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqMiddleware.h>
#include <unity/scopes/ScopeExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <condition_variable>
#include <mutex>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::zmq_middleware;

std::string const zmq_ini = TEST_DIR "/Zmq.ini";

TEST(PubSub, endpoints)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    auto publisher = mw.create_publisher("testpublisher");
    auto subscriber = mw.create_subscriber("testpublisher", "");

    EXPECT_EQ("ipc:///tmp/testpublisher-p", publisher->endpoint());
    EXPECT_EQ(subscriber->endpoint(), publisher->endpoint());
}

TEST(PubSub, exceptions)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    // Test that only one publisher can be created on a single endpoint
    auto publisher = mw.create_publisher("testpublisher");
    try
    {
        auto publisher2 = mw.create_publisher("testpublisher");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: ZmqPublisher(): publisher thread failed to start "
                     "(endpoint: ipc:///tmp/testpublisher-p):\n    unity::scopes::MiddlewareException: "
                     "safe_bind(): address in use: ipc:///tmp/testpublisher-p",
                     e.what());
    }

    // Test that multiple subscribers can be created on a single endpoint
    auto subscriber = mw.create_subscriber("testpublisher", "");
    try
    {
        auto subscriber2 = mw.create_subscriber("testpublisher", "testtopic1");
        auto subscriber3 = mw.create_subscriber("testpublisher", "testtopic2");
        auto subscriber4 = mw.create_subscriber("testpublisher", "testtopic3");
        auto subscriber5 = mw.create_subscriber("testpublisher", "testtopic4");
    }
    catch (MiddlewareException const& e)
    {
        FAIL();
    }

    // Test that an invalid publisher_id throws
    try
    {
        auto publisher2 = mw.create_publisher("/test/");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: ZmqPublisher(): A publisher cannot contain a '/' in its id",
                     e.what());
    }
    try
    {
        auto subscriber2 = mw.create_subscriber("/test/", "testtopic");
        FAIL();
    }
    catch (MiddlewareException const& e)
    {
        EXPECT_STREQ("unity::scopes::MiddlewareException: ZmqSubscriber(): A publisher cannot contain a '/' in its id",
                     e.what());
    }
}

class SubMsgReceiver
{
public:
    void receive1(std::string const& message)
    {
        last_sub_index_ = 1;
        last_message_ = message;
        received();
    }

    void receive2(std::string const& message)
    {
        last_sub_index_ = 2;
        last_message_ = message;
        received();
    }

    void receive3(std::string const& message)
    {
        last_sub_index_ = 3;
        last_message_ = message;
        received();
    }

    void receive4(std::string const& message)
    {
        last_sub_index_ = 4;
        last_message_ = message;
        received();
    }

    void received()
    {
        // Signal wait_for_message
        std::lock_guard<std::mutex> lock(mutex_);
        message_received_ = true;
        cond_.notify_one();
    }

    bool wait_for_message()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        bool message_received = cond_.wait_for(lock, std::chrono::milliseconds(500), [this] { return this->message_received_; });
        message_received_ = false;
        return message_received;
    }

    int last_sub_index_ = 0;
    std::string last_message_;

    bool message_received_ = false;
    std::mutex mutex_;
    std::condition_variable cond_;
};

TEST(PubSub, send_receive)
{
    SubMsgReceiver message_receiver;

    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    // Create 2 publishers
    auto publisher = mw.create_publisher("testpublisher");
    auto publisher2 = mw.create_publisher("testpublisher2");

    // Create a few subscribers
    auto subscriber1 = mw.create_subscriber("testpublisher", "testtopic1");
    subscriber1->message_received().connect(std::bind(&SubMsgReceiver::receive1, &message_receiver, placeholders::_1));

    auto subscriber2 = mw.create_subscriber("testpublisher", "testtopic2");
    subscriber2->message_received().connect(std::bind(&SubMsgReceiver::receive2, &message_receiver, placeholders::_1));

    auto subscriber3 = mw.create_subscriber("testpublisher", "");
    subscriber3->message_received().connect(std::bind(&SubMsgReceiver::receive3, &message_receiver, placeholders::_1));

    auto subscriber4 = mw.create_subscriber("testpublisher2", "testtopic4");
    subscriber4->message_received().connect(std::bind(&SubMsgReceiver::receive4, &message_receiver, placeholders::_1));

    // Give the subscribers some time to connect
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Send a "testtopic1" topic message (subscriber1)
    publisher->send_message("hello", "testtopic1");
    EXPECT_TRUE(message_receiver.wait_for_message());
    EXPECT_EQ(1, message_receiver.last_sub_index_);
    EXPECT_EQ("hello", message_receiver.last_message_);

    // Send a blank "testtopic2" topic message (subscriber2)
    publisher->send_message("", "testtopic2");
    EXPECT_TRUE(message_receiver.wait_for_message());
    EXPECT_EQ(2, message_receiver.last_sub_index_);
    EXPECT_EQ("", message_receiver.last_message_);

    // Send an empty topic message (subscriber3)
    publisher->send_message("hello again!", "");
    EXPECT_TRUE(message_receiver.wait_for_message());
    EXPECT_EQ(3, message_receiver.last_sub_index_);
    EXPECT_EQ("hello again!", message_receiver.last_message_);

    // Send a "testtopic4" topic message (subscriber4)
    // (no message should be received as subscriber4 is subscribed to a different publisher)
    publisher->send_message("test", "testtopic4");
    EXPECT_FALSE(message_receiver.wait_for_message());

    // Now send a "testtopic4" topic message from the correct publisher (subscriber4)
    publisher2->send_message("test", "testtopic4");
    EXPECT_TRUE(message_receiver.wait_for_message());
    EXPECT_EQ(4, message_receiver.last_sub_index_);
    EXPECT_EQ("test", message_receiver.last_message_);

    // Send a "unknown" topic message
    // (no message should be received as none of the subscribers are listening for "unknown")
    publisher->send_message("hello?", "unknown");
    EXPECT_FALSE(message_receiver.wait_for_message());

    publisher2->send_message("hello??", "unknown");
    EXPECT_FALSE(message_receiver.wait_for_message());
}

TEST(PubSub, threading)
{
    ZmqMiddleware mw("testscope", nullptr, zmq_ini);

    {
        // Create a publisher and a subscriber in seperate thread
        auto subscriber_future = std::async(std::launch::async, [&mw]{ return mw.create_subscriber("testpublisher", "testtopic"); });
        auto publisher_future = std::async(std::launch::async, [&mw]{ return mw.create_publisher("testpublisher"); });

        // Obtain the publisher and subscriber handles
        MWSubscriber::UPtr subscriber = subscriber_future.get();
        MWPublisher::UPtr publisher = publisher_future.get();
    }
    // The publisher and subscriber are now destroyed in this thread (should not hang / crash)
}
