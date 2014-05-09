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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <mutex>

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/MWScope.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>

#include "TestScope.h"

using namespace std;
using namespace unity::scopes;

class TestReceiver : public SearchListenerBase
{
public:
    TestReceiver()
        : query_complete_(false)
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
    }

    virtual void finished(ListenerBase::Reason reason, string const& error_message) override
    {
        lock_guard<mutex> lock(mutex_);
        reason_ = reason;
        error_message_ = error_message;
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
        query_complete_ = false;
    }

    ListenerBase::Reason reason()
    {
        lock_guard<mutex> lock(mutex_);
        return reason_;
    }

    string error_message()
    {
        lock_guard<mutex> lock(mutex_);
        return error_message_;
    }

private:
    bool query_complete_;
    ListenerBase::Reason reason_;
    string error_message_;
    mutex mutex_;
    condition_variable cond_;
};

// Check that invoking on a scope after a timeout exception from a previous
// invocation works correctly. This tests that a failed socket is removed
// from the connection pool in ZmqObject::invoke_twoway_().

TEST(Invocation, timeout)
{
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, rt.get(), "no_such_scope");

    auto receiver = make_shared<TestReceiver>();

    // First call must time out.
    scope->search("test", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(ListenerBase::Error, receiver->reason());
    EXPECT_EQ("unity::scopes::TimeoutException: Request timed out after 2000 milliseconds", receiver->error_message());

    // Wait another second, so TestScope is finally able to receive another request.
    this_thread::sleep_for(chrono::seconds(1));

    // Second call must succeed
    scope->search("test", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(ListenerBase::Finished, receiver->reason());
    EXPECT_EQ("", receiver->error_message());
}

void scope_thread(Runtime::SPtr const& rt, string const& runtime_ini_file)
{
    TestScope scope;
    rt->run_scope(&scope, runtime_ini_file, "");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    Runtime::SPtr srt = move(Runtime::create_scope_runtime("TestScope", "Runtime.ini"));
    std::thread scope_t(scope_thread, srt, "Runtime.ini");

    // Give thread some time to bind to its endpoint, to avoid getting ObjectNotExistException
    // from a synchronous remote call.
    this_thread::sleep_for(chrono::milliseconds(200));

    auto rc = RUN_ALL_TESTS();

    srt->destroy();
    scope_t.join();

    return rc;
}
