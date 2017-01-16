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

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/MWScope.h>
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "EmptyScope.h"
#include "TestScope.h"
#include "DebugTestScope.h"

#include <fstream>
#include <mutex>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

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

    virtual void finished(CompletionDetails const& details) override
    {
        lock_guard<mutex> lock(mutex_);
        status_ = details.status();
        error_message_ = details.message();
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
        query_complete_ = false;
    }

    CompletionDetails::CompletionStatus status()
    {
        lock_guard<mutex> lock(mutex_);
        return status_;
    }

    string error_message()
    {
        lock_guard<mutex> lock(mutex_);
        return error_message_;
    }

private:
    bool query_complete_;
    CompletionDetails::CompletionStatus status_;
    string error_message_;
    mutex mutex_;
    condition_variable cond_;
};

std::shared_ptr<core::posix::SignalTrap> trap(core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_chld}));
std::unique_ptr<core::posix::ChildProcess::DeathObserver> death_observer(core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(trap));

RuntimeImpl::SPtr run_test_registry()
{
    RuntimeImpl::SPtr runtime = RuntimeImpl::create("TestRegistry", "Runtime.ini");
    MiddlewareBase::SPtr middleware = runtime->factory()->create("TestRegistry", "Zmq", "Zmq.ini");
    RegistryObject::SPtr reg_obj(std::make_shared<RegistryObject>(*death_observer, std::make_shared<Executor>(), middleware));
    middleware->add_registry_object("TestRegistry", reg_obj);
    return runtime;
}

// Check that invoking on a scope after a timeout exception from a previous
// invocation works correctly. This tests that a failed socket is removed
// from the connection pool in ZmqObject::invoke_twoway_().

TEST(Invocation, timeout)
{
    auto reg_rt = run_test_registry();
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, "TestScope");

    auto receiver = make_shared<TestReceiver>();

    // First call must time out.
    scope->search("test", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::Error, receiver->status());
    string msg = receiver->error_message();
    boost::regex r("unity::scopes::TimeoutException: Request timed out after 500 milliseconds "
                   "\\(endpoint = .*TestScope\\, op = search\\)");
    EXPECT_TRUE(boost::regex_match(msg, r)) << msg;

    // Wait another three seconds, so TestScope is finally able to receive another request.
    this_thread::sleep_for(chrono::seconds(3));

    // Second call must succeed
    scope->search("test", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::OK, receiver->status());
    EXPECT_EQ("", receiver->error_message());
}

TEST(Invocation, no_timeout_in_debug_mode)
{
    auto reg_rt = run_test_registry();
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("DebugTestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("DebugTestScope");
    auto scope = internal::ScopeImpl::create(proxy, "TestScope");

    auto receiver = make_shared<TestReceiver>();

    // This call sleeps for 2s then returns
    scope->search("test", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    // Check that the two-way invocation timeout did not kick in due to "DebugMode = true"
    EXPECT_EQ(CompletionDetails::OK, receiver->status());
    EXPECT_EQ("", receiver->error_message());
}

class NullReceiver : public SearchListenerBase
{
public:
    NullReceiver()
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
    }

    virtual void finished(CompletionDetails const&) override
    {
    }
};

TEST(Invocation, shutdown_with_outstanding_async)
{
    auto reg_rt = run_test_registry();
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("EmptyScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("EmptyScope");
    auto scope = internal::ScopeImpl::create(proxy, "EmptyScope");

    auto receiver = make_shared<NullReceiver>();

    // Fire a bunch of searches and do *not* wait for them complete.
    // This tests that we correctly shut down the run time if there
    // are outstanding async invocations.
    for (int i = 0; i < 100; ++i)
    {
        scope->search("test", SearchMetadata("unused", "unused"), receiver);
    }
}

void testscope_thread(Runtime::SPtr const& rt)
{
    TestScope scope;
    rt->run_scope(&scope, "");
}

void nullscope_thread(Runtime::SPtr const& rt)
{
    EmptyScope scope;
    rt->run_scope(&scope, "");
}

void debugtestscope_thread(Runtime::SPtr const& rt)
{
    DebugTestScope scope;
    rt->run_scope(&scope, "DebugTestScope.ini");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    Runtime::SPtr tsrt = move(Runtime::create_scope_runtime("TestScope", "Runtime.ini"));
    std::thread testscope_t(testscope_thread, tsrt);

    Runtime::SPtr esrt = move(Runtime::create_scope_runtime("EmptyScope", "Runtime.ini"));
    std::thread emptyscope_t(nullscope_thread, esrt);

    Runtime::SPtr dsrt = move(Runtime::create_scope_runtime("DebugTestScope", "Runtime.ini"));
    std::thread debugtestscope_t(debugtestscope_thread, dsrt);

    // Give threads some time to bind to endpoints, to avoid getting a TimeoutException
    // from a synchronous remote call.
    this_thread::sleep_for(chrono::milliseconds(500));

    std::ifstream la("/proc/loadavg");
    std::string avg[3];
    la >> avg[0] >> avg[1] >> avg[2];
    std::cerr << "load average: " << avg[0] << " " << avg[1] << " " << avg[2] << std::endl;

    auto rc = RUN_ALL_TESTS();

    tsrt->destroy();
    testscope_t.join();

    esrt->destroy();
    emptyscope_t.join();

    dsrt->destroy();
    debugtestscope_t.join();

    return rc;
}
