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

#include "scope.h"

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Runtime, basic)
{
    Runtime::UPtr rt = Runtime::create("Runtime.ini");
    EXPECT_TRUE(rt->registry().get() != nullptr);
    rt->destroy();
}

class Receiver : public SearchListener
{
public:
    virtual void push(DepartmentList const& departments, std::string const& current_department_id) override
    {
        EXPECT_EQ(current_department_id, "news");
        EXPECT_EQ(1, departments.size());
        auto subdeps = departments.front().subdepartments();
        EXPECT_EQ(2, subdeps.size());
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
        EXPECT_EQ(1, annotation.links().size());
        EXPECT_EQ("Link1", annotation.links().front()->label());
        auto query = annotation.links().front()->query();
        EXPECT_EQ("scope-A", query.scope_name());
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

class PreviewReceiver : public PreviewListener
{
public:
    virtual void push(PreviewWidgetList const& widgets) override
    {
        EXPECT_EQ(widgets.size(), 2);
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

TEST(Runtime, create_query)
{
    // connect to scope and run a query
    auto rt = internal::RuntimeImpl::create("", "Runtime.ini");
    auto mw = rt->factory()->create("TestScope", "Zmq", "Zmq.ini");
    mw->start();
    auto proxy = mw->create_scope_proxy("TestScope");
    auto scope = internal::ScopeImpl::create(proxy, rt.get(), "TestScope");

    auto receiver = make_shared<Receiver>();
    auto ctrl = scope->create_query("test", SearchMetadata("en", "phone"), receiver);
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
    auto ctrl = scope->create_query("test", SearchMetadata("pl", "phone"), receiver);
    receiver->wait_until_finished();

    auto result = receiver->last_result();
    EXPECT_TRUE(result.get() != nullptr);

    auto target = result->target_scope_proxy();
    EXPECT_TRUE(target != nullptr);

    auto previewer = make_shared<PreviewReceiver>();
    auto preview_ctrl = target->preview(*(result.get()), ActionMetadata("en", "phone"), previewer);
    previewer->wait_until_finished();
}

void scope_thread(RuntimeImpl::SPtr const& rt)
{
    TestScope scope;
    rt->run_scope(&scope);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    RuntimeImpl::SPtr rt = move(RuntimeImpl::create("TestScope", "Runtime.ini"));
    std::thread scope_t(scope_thread, rt);
    auto rc = RUN_ALL_TESTS();
    rt->destroy();
    scope_t.join();
    return rc;
}
