/*
 * Copyright (C) 2015 Canonical Ltd
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
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/Runtime.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <condition_variable>
#include <thread>

#include "ThrowingScope.h"

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class Receiver : public SearchListenerBase
{
public:
    Receiver()
        : query_complete_(false)
    {
    }

    virtual void push(CategorisedResult result) override
    {
        lock_guard<mutex> lock(mutex_);

        results_.push_back(move(result));
    }

    virtual void finished(CompletionDetails const& details) override
    {
        lock_guard<mutex> lock(mutex_);

        completion_status_ = details.status();
        completion_msg_ = details.message();
        query_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);

        cond_.wait(lock, [this] { return this->query_complete_; });
        query_complete_ = false;
    }

    CompletionDetails::CompletionStatus completion_status()
    {
        lock_guard<mutex> lock(mutex_);

        return completion_status_;
    }

    string completion_msg()
    {
        lock_guard<mutex> lock(mutex_);

        return completion_msg_;
    }

    vector<CategorisedResult> results()
    {
        lock_guard<mutex> lock(mutex_);

        return results_;
    }

private:
    vector<CategorisedResult> results_;
    bool query_complete_;
    string completion_msg_;
    CompletionDetails::CompletionStatus completion_status_;
    mutex mutex_;
    condition_variable cond_;
};

class PreviewListener : public PreviewListenerBase
{
public:
    PreviewListener()
        : preview_complete_(false)
    {
    }

    void finished(const unity::scopes::CompletionDetails& details) override
    {
        lock_guard<mutex> lock(mutex_);

        completion_status_ = details.status();
        completion_msg_ = details.message();
        preview_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);

        cond_.wait(lock, [this] { return this->preview_complete_; });
        preview_complete_ = false;
    }

    CompletionDetails::CompletionStatus completion_status()
    {
        lock_guard<mutex> lock(mutex_);

        return completion_status_;
    }

    string completion_msg()
    {
        lock_guard<mutex> lock(mutex_);

        return completion_msg_;
    }

    void push(ColumnLayoutList const&) override
    {
    }

    void push(PreviewWidgetList const&) override
    {
    }

    void push(std::string const&, Variant const&) override
    {
    }

private:
    bool preview_complete_;
    string completion_msg_;
    CompletionDetails::CompletionStatus completion_status_;
    mutex mutex_;
    condition_variable cond_;
};

class ActivationListener : public ActivationListenerBase
{
public:
    ActivationListener()
        : activate_complete_(false)
    {
    }

    void activated(ActivationResponse const& response) override
    {
        lock_guard<mutex> lock(mutex_);

        act_status_ = response.status();
    }

    void finished(const unity::scopes::CompletionDetails& details) override
    {
        lock_guard<mutex> lock(mutex_);

        completion_status_ = details.status();
        completion_msg_ = details.message();
        activate_complete_ = true;
        cond_.notify_one();
    }

    void wait_until_finished()
    {
        unique_lock<mutex> lock(mutex_);

        cond_.wait(lock, [this] { return this->activate_complete_; });
        activate_complete_ = false;
    }

    CompletionDetails::CompletionStatus completion_status()
    {
        lock_guard<mutex> lock(mutex_);

        return completion_status_;
    }

    string completion_msg()
    {
        lock_guard<mutex> lock(mutex_);

        return completion_msg_;
    }

private:
    ActivationResponse::Status act_status_;
    bool activate_complete_;
    string completion_msg_;
    CompletionDetails::CompletionStatus completion_status_;
    mutex mutex_;
    condition_variable cond_;
};

class ThrowingScopeTest : public ::testing::Test
{
public:
    ThrowingScopeTest()
    {
        runtime_ = Runtime::create(TEST_RUNTIME_FILE);
        auto reg = runtime_->registry();
        auto meta = reg->get_metadata("ThrowingScope");
        scope_ = meta.proxy();
    }

    ScopeProxy scope() const
    {
        return scope_;
    }

private:
    Runtime::UPtr runtime_;
    ScopeProxy scope_;
};

TEST_F(ThrowingScopeTest, no_error)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());
}

TEST_F(ThrowingScopeTest, throw_from_search)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from search", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("unity::scopes::MiddlewareException: unity::ResourceException: Scope \"ThrowingScope\" "
                  "threw an exception from search():\n"
                  "    unity::ResourceException: exception from search",
              receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(0u, r.size());
}

TEST_F(ThrowingScopeTest, throw_from_run)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from run", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("QueryBase::run(): unity::ResourceException: exception from run", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(0u, r.size());
}

TEST_F(ThrowingScopeTest, throw_from_cancelled)
{
    auto receiver = make_shared<Receiver>();
    auto ctrl = scope()->search("throw from cancelled", SearchMetadata("unused", "unused"), receiver);
    // Make sure that the cancel goes to the scope instead of being cached locally.
    this_thread::sleep_for(chrono::seconds(1));
    ctrl->cancel();
    receiver->wait_until_finished();

    auto r = receiver->results();
    EXPECT_EQ(0u, r.size());

    EXPECT_EQ(CompletionDetails::CompletionStatus::Cancelled, receiver->completion_status());
    // No exception string here because we short-cut the cancellation on the client side.
    // The exception is logged though.
    EXPECT_EQ("", receiver->completion_msg());
}

TEST_F(ThrowingScopeTest, throw_from_preview)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from preview", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());

    auto plistener = make_shared<PreviewListener>();
    scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, plistener->completion_status());
    EXPECT_EQ("unity::scopes::MiddlewareException: unity::ResourceException: Scope \"ThrowingScope\" "
                  "threw an exception from preview():\n"
                  "    unity::ResourceException: exception from preview",
              plistener->completion_msg());
}

TEST_F(ThrowingScopeTest, throw_from_preview_run)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from preview run", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());

    auto plistener = make_shared<PreviewListener>();
    scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, plistener->completion_status());
    EXPECT_EQ("PreviewQueryBase::run(): unity::ResourceException: throw from preview run", plistener->completion_msg());
}

TEST_F(ThrowingScopeTest, throw_from_preview_cancelled)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from preview cancelled", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());

    auto plistener = make_shared<PreviewListener>();
    auto ctrl = scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    this_thread::sleep_for(chrono::seconds(1));
    ctrl->cancel();
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Cancelled, plistener->completion_status());
    // No exception string here because we short-cut the cancellation on the client side.
    // The exception is logged though.
    EXPECT_EQ("", plistener->completion_msg());
}

TEST_F(ThrowingScopeTest, throw_from_activate)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from activate", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());

    auto alistener = make_shared<ActivationListener>();
    scope()->activate(r[0], ActionMetadata("unused", "unused"), alistener);
    alistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, alistener->completion_status());
    EXPECT_EQ("unity::scopes::MiddlewareException: unity::ResourceException: Scope \"ThrowingScope\" "
                  "threw an exception from activate():\n"
                  "    unity::ResourceException: exception from activate",
              alistener->completion_msg());
}

TEST_F(ThrowingScopeTest, throw_from_activation_activate)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from activation activate", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());

    auto alistener = make_shared<ActivationListener>();
    scope()->activate(r[0], ActionMetadata("unused", "unused"), alistener);
    alistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, alistener->completion_status());
    EXPECT_EQ("ActivationQueryBase::activate(): unity::ResourceException: throw from activation activate",
              alistener->completion_msg());
}

TEST_F(ThrowingScopeTest, throw_from_activation_cancelled)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from activation cancelled", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());

    auto alistener = make_shared<ActivationListener>();
    auto ctrl = scope()->activate(r[0], ActionMetadata("unused", "unused"), alistener);
    this_thread::sleep_for(chrono::seconds(1));
    ctrl->cancel();
    alistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Cancelled, alistener->completion_status());
    // No exception string here because we short-cut the cancellation on the client side.
    // The exception is logged though.
    EXPECT_EQ("", alistener->completion_msg());
}

TEST_F(ThrowingScopeTest, throw_from_perform_action)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from perform_action", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());

    auto alistener = make_shared<ActivationListener>();
    scope()->perform_action(r[0], ActionMetadata("unused", "unused"), "", "", alistener);
    alistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, alistener->completion_status());
    EXPECT_EQ("unity::scopes::MiddlewareException: unity::ResourceException: Scope \"ThrowingScope\" "
                  "threw an exception from perform_action():\n"
                  "    unity::ResourceException: exception from perform_action",
              alistener->completion_msg());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;

    // Set the "TEST_DESKTOP_FILES_DIR" env var before forking as not to create desktop files in ~/.local
    putenv(const_cast<char*>("TEST_DESKTOP_FILES_DIR=" TEST_RUNTIME_PATH));

    auto rpid = fork();
    if (rpid == 0)
    {
        const char* const args[] = {"scoperegistry [ThrowingScope_test]", TEST_RUNTIME_FILE, nullptr};
        if (execv(TEST_REGISTRY_PATH "/scoperegistry", const_cast<char* const*>(args)) < 0)
        {
            perror("Error starting scoperegistry:");
        }
        return 1;
    }
    else if (rpid > 0)
    {
        rc = RUN_ALL_TESTS();
        kill(rpid, SIGTERM);
        waitpid(rpid, nullptr, 0);
    }
    else
    {
        perror("Failed to fork:");
    }

    return rc;
}
