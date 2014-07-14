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

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>

#include <gtest/gtest.h>

#include "SlowSearchScope.h"

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class NullReceiver : public SearchListenerBase
{
public:
    NullReceiver()
        : query_complete_(false)
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
    }

    virtual void finished(ListenerBase::Reason reason, string const& error_message) override
    {
        EXPECT_EQ(ListenerBase::Reason::Error, reason);
        EXPECT_EQ("unity::scopes::MiddlewareException: unity::scopes::MiddlewareException: "
                  "Cannot invoke operations while middleware is stopped",
                  error_message);
        lock_guard<mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
        query_complete_ = false;
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable cond_;
};

void slowsearchscope_thread(RuntimeImpl::SPtr const& rt, string const& runtime_ini_file)
{
    SlowSearchScope scope;
    rt->run_scope(&scope, runtime_ini_file, "SlowSearchScope.ini");
}

// Check that the idle timeout for a server works if no operations are in progress

TEST(IdleTimeout, server_idle_timeout_while_idle)
{
    auto start_time = chrono::steady_clock::now();

    {
        SlowSearchScope scope;

        // Make a run time for the scope and run the scope.
        auto rt = RuntimeImpl::create("SlowSearchScope", "Runtime.ini");
        rt->run_scope(&scope, "Runtime.ini", "SlowSearchScope.ini");
    }

    // Check that the scope has indeed timed out. The server shuts down after 2 seconds,
    // so we allow between 2 and 6 seconds for it to time out.
    auto duration = chrono::steady_clock::now() - start_time;
    EXPECT_TRUE(duration > chrono::seconds(2));
    EXPECT_TRUE(duration < chrono::seconds(6));
}

// Check that the idle timeout for a server waits for synchronous operations
// to complete.

TEST(IdleTimeout, server_idle_timeout_while_operation_in_progress)
{
    auto start_time = chrono::steady_clock::now();

    {
        // Make a run time for the scope and run the scope.
        RuntimeImpl::SPtr srt = RuntimeImpl::create("SlowSearchScope", "Runtime.ini");
        std::thread slowsearchscope_t(slowsearchscope_thread, srt, "Runtime.ini");

        // Give scope some time to bind to endpoint.
        this_thread::sleep_for(chrono::milliseconds(200));

        // Make a client run time.
        auto rt = RuntimeImpl::create("", "Runtime.ini");

        // Make a proxy for the scope
        auto mw = rt->factory()->create("SlowSearchScope", "Zmq", "Zmq.ini");
        mw->start();
        auto proxy = mw->create_scope_proxy("SlowSearchScope");
        auto scope = internal::ScopeImpl::create(proxy, rt.get(), "SlowSearchScope");

        // Create a query, which takes 4 seconds to complete in the scope.
        auto receiver = make_shared<NullReceiver>();
        scope->search("test", SearchMetadata("unused", "unused"), receiver);
        receiver->wait_until_finished();

        // Scope thread will exit once synchronous operation completes.
        slowsearchscope_t.join();
    }

    // Check that the run time doesn't shut down until
    // the search in the scope has completed, which takes 4 seconds.
    // We allow 4 - 11 seconds for things to shut down. Typically,
    // the finished() message will be received just after the
    // search operation completes in the scope. However, occasionally
    // the finished() message can get lost, depending on timing
    // of the closure of the outgoing socket in the server for oneway
    // invocations (which we cannot control). In that case, the test
    // terminates after 10 seconds, when the client-side idle timeout
    // expires. This is harmless: a scope should not take longer than
    // the server idle timeout to respond to a search request.
    auto duration = chrono::steady_clock::now() - start_time;
    EXPECT_TRUE(duration > chrono::seconds(4));
    if (duration > chrono::seconds(6))
    {
        cerr << "finished() message not received after server timeout." << endl;
        cerr << "Test finished due to client-side timeout." << endl;
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    auto rc = RUN_ALL_TESTS();

    return rc;
}
