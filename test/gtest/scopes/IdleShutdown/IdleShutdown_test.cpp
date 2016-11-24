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
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "SlowSearchScope.h"

#include <fstream>

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

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::Error, details.status());
        EXPECT_EQ("unity::scopes::MiddlewareException: unity::scopes::MiddlewareException: "
                  "Cannot invoke operations while middleware is stopped",
                  details.message());
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

void slowsearchscope_thread(RuntimeImpl::SPtr const& rt)
{
    SlowSearchScope scope;
    rt->run_scope(&scope, "SlowSearchScope.ini");
}

// Check that the idle timeout for a server works if no operations are in progress

TEST(IdleTimeout, server_idle_timeout_while_idle)
{
    auto start_time = chrono::steady_clock::now();

    {
        SlowSearchScope scope;

        // Make a run time for the scope and run the scope.
        auto rt = RuntimeImpl::create("SlowSearchScope", "Runtime.ini");
        rt->run_scope(&scope, "SlowSearchScope.ini");
    }

    // Check that the scope has indeed timed out. The server shuts down after 2 seconds,
    // so we allow between 2 and 7 seconds for it to time out.
    auto duration = chrono::steady_clock::now() - start_time;
    EXPECT_TRUE(duration > chrono::seconds(2));
    EXPECT_TRUE(duration < chrono::seconds(7));
}

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

// Check that the idle timeout for a server waits for synchronous operations
// to complete.

TEST(IdleTimeout, server_idle_timeout_while_operation_in_progress)
{
    auto reg_rt = run_test_registry();
    auto start_time = chrono::steady_clock::now();

    {
        // Make a run time for the scope and run the scope.
        RuntimeImpl::SPtr srt = RuntimeImpl::create("SlowSearchScope", "Runtime.ini");
        std::thread slowsearchscope_t(slowsearchscope_thread, srt);

        // Give scope some time to bind to endpoint.
        this_thread::sleep_for(chrono::milliseconds(200));

        // Make a client run time.
        auto rt = RuntimeImpl::create("", "Runtime.ini");

        // Make a proxy for the scope
        auto mw = rt->factory()->create("client_middleware", "Zmq", "Zmq.ini");
        mw->start();
        auto proxy = mw->create_scope_proxy("SlowSearchScope");
        auto scope = internal::ScopeImpl::create(proxy, "SlowSearchScope");

        // Create a query, which takes 4 seconds to complete in the scope.
        auto receiver = make_shared<NullReceiver>();
        scope->search("test", SearchMetadata("unused", "unused"), receiver);
        receiver->wait_until_finished();

        // Scope thread will exit once synchronous operation completes.
        slowsearchscope_t.join();
    }

    // We allow 4 - 8 seconds for things to shut down.
    // If no finished message has arrived by then, something is broken.
    auto duration = chrono::steady_clock::now() - start_time;
    EXPECT_GT(duration, chrono::seconds(4));
    EXPECT_LT(duration, chrono::seconds(8));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    std::ifstream la("/proc/loadavg");
    std::string avg[3];
    la >> avg[0] >> avg[1] >> avg[2];
    std::cerr << "load average: " << avg[0] << " " << avg[1] << " " << avg[2] << std::endl;

    return RUN_ALL_TESTS();
}
