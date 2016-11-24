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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "PusherScope.h"
#include "SlowCreateScope.h"
#include "TestScope.h"

#include <fstream>
#include <mutex>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Runtime, basic)
{
    Runtime::UPtr rt = Runtime::create("Runtime.ini");
    EXPECT_TRUE(rt->registry().get() != nullptr);
    rt->destroy();
}

class Receiver : public SearchListenerBase
{
public:
    Receiver() :
        query_complete_(false),
        count_(0),
        dep_count_(0),
        annotation_count_(0),
        info_count_(0)
    {
    }

    virtual void push(Department::SCPtr const& parent) override
    {
        EXPECT_EQ(parent->id(), "");
        auto subdeps = parent->subdepartments();
        EXPECT_EQ(1u, subdeps.size());
        auto current = *subdeps.begin();
        EXPECT_EQ(current->id(), "news");
        subdeps = current->subdepartments();
        EXPECT_EQ(2u, subdeps.size());
        EXPECT_EQ("subdep1", subdeps.front()->id());
        EXPECT_EQ("Europe", subdeps.front()->label());
        EXPECT_EQ("test", subdeps.front()->query().query_string());
        EXPECT_EQ("subdep2", subdeps.back()->id());
        EXPECT_EQ("US", subdeps.back()->label());
        EXPECT_EQ("test", subdeps.back()->query().query_string());
        dep_count_++;
    }

    virtual void push(CategorisedResult result) override
    {
        EXPECT_EQ("uri", result.uri());
        EXPECT_EQ("title", result.title());
        EXPECT_EQ("art", result.art());
        EXPECT_EQ("dnd_uri", result.dnd_uri());
        count_++;
        lock_guard<mutex> lock(mutex_);
        last_result_ = std::make_shared<Result>(result);
    }

    virtual void push(experimental::Annotation annotation) override
    {
        EXPECT_EQ(1u, annotation.links().size());
        EXPECT_EQ("Link1", annotation.links().front()->label());
        auto query = annotation.links().front()->query();
        EXPECT_EQ("scope-A", query.scope_id());
        EXPECT_EQ("foo", query.query_string());
        EXPECT_EQ("dep1", query.department_id());
        annotation_count_++;
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::OK, details.status());
        EXPECT_EQ("", details.message());
        EXPECT_EQ(1, count_);
        EXPECT_EQ(1, dep_count_);
        EXPECT_EQ(1, annotation_count_);
        EXPECT_EQ(2, info_count_);

        EXPECT_EQ(2u, details.info_list().size());
        if (details.info_list().size() == 2)
        {
            EXPECT_EQ(OperationInfo::NoInternet, details.info_list()[0].code());
            EXPECT_EQ("Partial results returned due to no internet connection.", details.info_list()[0].message());
            EXPECT_EQ(OperationInfo::PoorInternet, details.info_list()[1].code());
            EXPECT_EQ("Partial results returned due to poor internet connection.", details.info_list()[1].message());
        }

        // Signal that the query has completed.
        unique_lock<mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

    virtual void info(OperationInfo const& op_info) override
    {
        if (info_count_ == 0)
        {
            EXPECT_EQ(OperationInfo::NoInternet, op_info.code());
            EXPECT_EQ("Partial results returned due to no internet connection.", op_info.message());
        }
        else if (info_count_ == 1)
        {
            EXPECT_EQ(OperationInfo::PoorInternet, op_info.code());
            EXPECT_EQ("Partial results returned due to poor internet connection.", op_info.message());
        }
        info_count_++;
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);
        cond_.wait(lock, [this] { return this->query_complete_; });
    }

    std::shared_ptr<Result> last_result()
    {
        lock_guard<mutex> lock(mutex_);
        return last_result_;
    }

private:
    bool query_complete_;
    mutex mutex_;
    condition_variable cond_;
    atomic_int count_;
    atomic_int dep_count_;
    atomic_int annotation_count_;
    atomic_int info_count_;
    std::shared_ptr<Result> last_result_;
};

class PreviewReceiver : public PreviewListenerBase
{
public:
    PreviewReceiver() :
        query_complete_(false),
        widgets_pushes_(0),
        data_pushes_(0),
        info_count_(0)
    {
    }

    virtual void push(PreviewWidgetList const& widgets) override
    {
        EXPECT_EQ(2u, widgets.size());
        widgets_pushes_++;
    }

    virtual void push(std::string const& key, Variant const&) override
    {
        EXPECT_TRUE(key == "author" || key == "rating");
        data_pushes_++;
    }

    virtual void push(ColumnLayoutList const&) override
    {
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::OK, details.status());
        EXPECT_EQ("", details.message());
        EXPECT_EQ(1, widgets_pushes_);
        EXPECT_EQ(2, data_pushes_);
        EXPECT_EQ(2, info_count_);

        EXPECT_EQ(2u, details.info_list().size());
        if (details.info_list().size() == 2)
        {
            EXPECT_EQ(OperationInfo::NoLocationData, details.info_list()[0].code());
            EXPECT_EQ("", details.info_list()[0].message());
            EXPECT_EQ(OperationInfo::InaccurateLocationData, details.info_list()[1].code());
            EXPECT_EQ("Partial results returned due to inaccurate location data.", details.info_list()[1].message());
        }

        // Signal that the query has completed.
        unique_lock<mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
    }

    virtual void info(OperationInfo const& op_info) override
    {
        if (info_count_ == 0)
        {
            EXPECT_EQ(OperationInfo::NoLocationData, op_info.code());
            EXPECT_EQ("", op_info.message());
        }
        else if (info_count_ == 1)
        {
            EXPECT_EQ(OperationInfo::InaccurateLocationData, op_info.code());
            EXPECT_EQ("Partial results returned due to inaccurate location data.", op_info.message());
        }
        info_count_++;
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
    atomic_int widgets_pushes_;
    atomic_int data_pushes_;
    atomic_int info_count_;
};

class PushReceiver : public SearchListenerBase
{
public:
    PushReceiver(int pushes_expected)
        : query_complete_(false),
          pushes_expected_(pushes_expected),
          count_(0)
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
        if (++count_ > pushes_expected_)
        {
            FAIL();
        }
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::OK, details.status()) << details.message();
        EXPECT_EQ(pushes_expected_, count_);
        // Signal that the query has completed.
        unique_lock<mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
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
    atomic_int pushes_expected_;
    atomic_int count_;
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

TEST(Runtime, search)
{
    auto reg_rt = run_test_registry();

    // connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, "TestScope");

    auto receiver = make_shared<Receiver>();
    auto ctrl = scope->search("test", SearchMetadata("en", "phone"), receiver);
    receiver->wait_until_finished();
}

TEST(Runtime, consecutive_search)
{
    auto reg_rt = run_test_registry();
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();

    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, "TestScope");

    auto receiver = make_shared<Receiver>();
    auto ctrl = scope->search("test", SearchMetadata("en", "phone"), receiver);
    receiver->wait_until_finished();

    std::vector<std::shared_ptr<Receiver>> replies;

    const int num_searches = 100;
    for (int i = 0; i < num_searches; ++i)
    {
        replies.push_back(std::make_shared<Receiver>());
        scope->search("test", SearchMetadata("en", "phone"), replies.back());
    }

    for (int i = 0; i < num_searches; ++i)
    {
        replies[i]->wait_until_finished();
    }

    // Do it again, to test that re-use of previously-created connections works.
    replies.clear();
    for (int i = 0; i < num_searches; ++i)
    {
        replies.push_back(std::make_shared<Receiver>());
        scope->search("test", SearchMetadata("en", "phone"), replies.back());
    }

    for (int i = 0; i < num_searches; ++i)
    {
        replies[i]->wait_until_finished();
    }
}

TEST(Runtime, preview)
{
    auto reg_rt = run_test_registry();

    // connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, "TestScope");

    // run a query first, so we have a result to preview
    auto receiver = make_shared<Receiver>();
    auto ctrl = scope->search("test", SearchMetadata("pl", "phone"), receiver);
    receiver->wait_until_finished();

    auto result = receiver->last_result();
    EXPECT_TRUE(result.get() != nullptr);

    if (result.get() != nullptr)
    {
        auto target = result->target_scope_proxy();
        EXPECT_TRUE(target != nullptr);
        auto previewer = make_shared<PreviewReceiver>();
        auto preview_ctrl = target->preview(*(result.get()), ActionMetadata("en", "phone"), previewer);
        previewer->wait_until_finished();
    }
}

TEST(Runtime, cardinality)
{
    auto reg_rt = run_test_registry();

    // connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("PusherScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("PusherScope");
    auto scope = internal::ScopeImpl::create(proxy, "PusherScope");

    // Run a query with unlimited cardinality. We check that the
    // scope returns 100 results.
    auto receiver = make_shared<PushReceiver>(100);
    scope->search("test", SearchMetadata(100, "unused", "unused"), receiver);
    receiver->wait_until_finished();

    // Run a query with 20 cardinality. We check that we receive only 20 results and,
    // in the scope, check that push() returns true for the first 19, and false afterwards.
    receiver = make_shared<PushReceiver>(20);
    scope->search("test", SearchMetadata(20, "unused", "unused"), receiver);
    receiver->wait_until_finished();
}

class CancelReceiver : public SearchListenerBase
{
public:
    CancelReceiver()
        : query_complete_(false)
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
        FAIL();
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::Cancelled, details.status());
        EXPECT_EQ("", details.message());
        // Signal that the query has completed.
        unique_lock<mutex> lock(mutex_);
        query_complete_ = true;
        cond_.notify_one();
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

TEST(Runtime, early_cancel)
{
    auto reg_rt = run_test_registry();
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("SlowCreateScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("SlowCreateScope");
    auto scope = internal::ScopeImpl::create(proxy, "SlowCreateScope");

    // Allow some time for the middleware to start up.
    this_thread::sleep_for(chrono::milliseconds(200));

    // Check that, if a cancel is sent before search() returns on the server side, the
    // cancel is correctly forwarded to the scope once the real reply proxy arrives
    // over the wire.
    auto receiver = make_shared<CancelReceiver>();
    auto ctrl = scope->search("test", SearchMetadata("unused", "unused"), receiver);
    // Allow some time for the search message to get there.
    this_thread::sleep_for(chrono::milliseconds(100));

    // search() in the scope doesn't return for some time, so the cancel() that follows
    // is sent to the "fake" QueryCtrlProxy.
    ctrl->cancel();
    receiver->wait_until_finished();

    // The receiver receives its cancel from the client-side run time instead of the
    // scope because the run time short-cuts sending the cancel locally instead
    // of waiting for the cancel message from the scope. Allow some time for the
    // cancel to reach the scope before shutting down the run time, so the scope
    // can test that it received the cancel.
    this_thread::sleep_for(chrono::milliseconds(500));
}

void scope_thread(Runtime::SPtr const& rt)
{
    TestScope scope;
    rt->run_scope(&scope, "");
}

void pusher_thread(Runtime::SPtr const& rt)
{
    PusherScope scope;
    rt->run_scope(&scope, "");
}

void slow_create_thread(Runtime::SPtr const& rt)
{
    SlowCreateScope scope;
    rt->run_scope(&scope, "");
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;

    Runtime::SPtr srt = move(Runtime::create_scope_runtime("TestScope", "Runtime.ini"));
    std::thread scope_t(scope_thread, srt);

    Runtime::SPtr prt = move(Runtime::create_scope_runtime("PusherScope", "Runtime.ini"));
    std::thread pusher_t(pusher_thread, prt);

    Runtime::SPtr scrt = move(Runtime::create_scope_runtime("SlowCreateScope", "Runtime.ini"));
    std::thread slow_create_t(slow_create_thread, scrt);

    // Give threads some time to bind to their endpoints, to avoid getting ObjectNotExistException
    // from a synchronous remote call.
    this_thread::sleep_for(chrono::milliseconds(500));

    std::ifstream la("/proc/loadavg");
    std::string avg[3];
    la >> avg[0] >> avg[1] >> avg[2];
    std::cerr << "load average: " << avg[0] << " " << avg[1] << " " << avg[2] << std::endl;

    rc = RUN_ALL_TESTS();

    srt->destroy();
    scope_t.join();

    prt->destroy();
    pusher_t.join();

    scrt->destroy();
    slow_create_t.join();

    return rc;
}
