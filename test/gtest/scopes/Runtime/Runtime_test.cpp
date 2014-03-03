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

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include <mutex>

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/MWScope.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>

#include "TestScope.h"
#include "PusherScope.h"

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
        annotation_count_(0)
    {
    }

    virtual void push(DepartmentList const& departments, std::string const& current_department_id) override
    {
        EXPECT_EQ(current_department_id, "news");
        EXPECT_EQ(1u, departments.size());
        auto subdeps = departments.front().subdepartments();
        EXPECT_EQ(2u, subdeps.size());
        EXPECT_EQ("subdep1", subdeps.front().id());
        EXPECT_EQ("Europe", subdeps.front().label());
        EXPECT_EQ("test", subdeps.front().query().query_string());
        EXPECT_EQ("subdep2", subdeps.back().id());
        EXPECT_EQ("US", subdeps.back().label());
        EXPECT_EQ("test", subdeps.back().query().query_string());
        dep_count_++;
    }

    virtual void push(CategorisedResult result) override
    {
        EXPECT_EQ("uri", result.uri());
        EXPECT_EQ("title", result.title());
        EXPECT_EQ("art", result.art());
        EXPECT_EQ("dnd_uri", result.dnd_uri());
        count_++;
        last_result_ = std::make_shared<Result>(result);
    }
    virtual void push(Annotation annotation) override
    {
        EXPECT_EQ(1u, annotation.links().size());
        EXPECT_EQ("Link1", annotation.links().front()->label());
        auto query = annotation.links().front()->query();
        EXPECT_EQ("scope-A", query.scope_id());
        EXPECT_EQ("foo", query.query_string());
        EXPECT_EQ("dep1", query.department_id());
        annotation_count_++;
    }
    virtual void finished(ListenerBase::Reason reason, string const& error_message) override
    {
        EXPECT_EQ(Finished, reason);
        EXPECT_EQ("", error_message);
        EXPECT_EQ(1, count_);
        EXPECT_EQ(1, dep_count_);
        EXPECT_EQ(1, annotation_count_);
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
    std::shared_ptr<Result> last_result()
    {
        return last_result_;
    }
private:
    bool query_complete_;
    mutex mutex_;
    condition_variable cond_;
    int count_;
    int dep_count_;
    int annotation_count_;
    std::shared_ptr<Result> last_result_;
};

class PreviewReceiver : public PreviewListenerBase
{
public:
    PreviewReceiver() :
        query_complete_(false),
        widgets_pushes_(0),
        data_pushes_(0)
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
    virtual void finished(ListenerBase::Reason reason, string const& error_message) override
    {
        EXPECT_EQ(Finished, reason);
        EXPECT_EQ("", error_message);
        EXPECT_EQ(1, widgets_pushes_);
        EXPECT_EQ(2, data_pushes_);
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
    int widgets_pushes_;
    int data_pushes_;
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

    virtual void finished(ListenerBase::Reason reason, string const& /* error_message */) override
    {
        EXPECT_EQ(Finished, reason);
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

TEST(Runtime, search)
{
    // connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, rt.get(), "TestScope");

    auto receiver = make_shared<Receiver>();
    auto ctrl = scope->search("test", SearchMetadata("en", "phone"), receiver);
    receiver->wait_until_finished();
}

TEST(Runtime, preview)
{
    // connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, rt.get(), "TestScope");

    // run a query first, so we have a result to preview
    auto receiver = make_shared<Receiver>();
    auto ctrl = scope->search("test", SearchMetadata("pl", "phone"), receiver);
    receiver->wait_until_finished();

    auto result = receiver->last_result();
    EXPECT_TRUE(result.get() != nullptr);

    auto target = result->target_scope_proxy();
    EXPECT_TRUE(target != nullptr);

    auto previewer = make_shared<PreviewReceiver>();
    auto preview_ctrl = target->preview(*(result.get()), ActionMetadata("en", "phone"), previewer);
    previewer->wait_until_finished();
}

TEST(Runtime, cardinality)
{
    // connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("PusherScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("PusherScope");
    auto scope = internal::ScopeImpl::create(proxy, rt.get(), "PusherScope");

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

void scope_thread(RuntimeImpl::SPtr const& rt)
{
    TestScope scope;
    rt->run_scope(&scope);
}

void pusher_thread(RuntimeImpl::SPtr const& rt)
{
    PusherScope scope;
    rt->run_scope(&scope);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    RuntimeImpl::SPtr srt = move(RuntimeImpl::create("TestScope", "Runtime.ini"));
    std::thread scope_t(scope_thread, srt);

    RuntimeImpl::SPtr prt = move(RuntimeImpl::create("PusherScope", "Runtime.ini"));
    std::thread pusher_t(pusher_thread, prt);

    auto rc = RUN_ALL_TESTS();

    srt->destroy();
    scope_t.join();

    prt->destroy();
    pusher_t.join();

    return rc;
}
