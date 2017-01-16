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

#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>

#include <boost/algorithm/string.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "NoReplyScope.h"
#include <stdexcept>

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
        // Check that finished() was called by the reaper.
        EXPECT_EQ(CompletionDetails::Error, details.status()) << details.message();
        EXPECT_TRUE(boost::starts_with(details.message(), "No activity on ReplyObject for scope"));
        lock_guard<mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_all();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable cond_;
};

std::shared_ptr<core::posix::SignalTrap>
    trap(core::posix::trap_signals_for_all_subsequent_threads({core::posix::Signal::sig_chld}));
std::unique_ptr<core::posix::ChildProcess::DeathObserver>
    death_observer(core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(trap));

RuntimeImpl::SPtr run_test_registry()
{
    RuntimeImpl::SPtr runtime = RuntimeImpl::create("TestRegistry", TEST_DIR "/Runtime.ini");
    MiddlewareBase::SPtr middleware = runtime->factory()->create("TestRegistry", "Zmq", "Zmq.ini");
    RegistryObject::SPtr reg_obj(std::make_shared<RegistryObject>(*death_observer, std::make_shared<Executor>(), middleware));
    middleware->add_registry_object("TestRegistry", reg_obj);
    return runtime;
}

void scope_thread(Runtime::SPtr const& rt)
{
    NoReplyScope scope;
    rt->run_scope(&scope, "");
}

TEST(ReplyReaper, reap)
{
    auto reg_rt = run_test_registry();

    Runtime::SPtr no_reply_rt = move(Runtime::create_scope_runtime("NoReplyScope", TEST_DIR "/Runtime.ini"));
    std::thread scope_t(scope_thread, no_reply_rt);

    try
    {
        // Run a query in the scope. The query will do nothing for 3 seconds,
        // but the reaper will reap after at most 2 seconds.
        auto rt = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto mw = rt->factory()->create("NoReplyScope", "Zmq", TEST_DIR "/Zmq.ini");
        mw->start();
        auto proxy = mw->create_scope_proxy("NoReplyScope");
        auto scope = internal::ScopeImpl::create(proxy, "NoReplyScope");

        auto receiver = make_shared<NullReceiver>();
        scope->search("test", SearchMetadata("en", "phone"), receiver);
        receiver->wait_until_finished();

        no_reply_rt->destroy();
        scope_t.join();
    }
    catch (std::exception const& e)
    {
        no_reply_rt->destroy();
        scope_t.join();
        FAIL() << e.what();
    }
}

class NoReapReceiver : public SearchListenerBase
{
public:
    NoReapReceiver()
        : query_complete_(false)
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
    }

    virtual void finished(CompletionDetails const& details) override
    {
        // Check that finished() was called by the reaper.
        EXPECT_EQ(CompletionDetails::OK, details.status());
        EXPECT_EQ("", details.message());
        lock_guard<mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_all();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable cond_;
};

void scope_thread_debug_mode(Runtime::SPtr const& rt)
{
    NoReplyScope scope;
    rt->run_scope(&scope, TEST_DIR "/DebugScope.ini");
}

TEST(ReplyReaper, no_reap_in_debug_mode)
{
    auto reg_rt = run_test_registry();

    Runtime::SPtr no_reply_rt = move(Runtime::create_scope_runtime("NoReplyScope", TEST_DIR "/Runtime.ini"));
    std::thread scope_t(scope_thread_debug_mode, no_reply_rt);

    try
    {
        // Run a query in the scope. The query will do nothing for 3 seconds,
        // but the reaper will reap after at most 2 seconds.
        auto rt = internal::RuntimeImpl::create("", TEST_DIR "/Runtime.ini");
        auto mw = rt->factory()->create("NoReplyScope", "Zmq", TEST_DIR "/Zmq.ini");
        mw->start();
        auto proxy = mw->create_scope_proxy("NoReplyScope");
        auto scope = internal::ScopeImpl::create(proxy, "NoReplyScope");

        auto receiver = make_shared<NoReapReceiver>();
        scope->search("test", SearchMetadata("en", "phone"), receiver);
        receiver->wait_until_finished();

        no_reply_rt->destroy();
        scope_t.join();
    }
    catch (std::exception const& e)
    {
        no_reply_rt->destroy();
        scope_t.join();
        FAIL() << e.what();
    }
}
