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
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/QueryCtrl.h>

#include <gtest/gtest.h>

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

// The command file for each scope is called A.cmd, B.cmd, etc. It contains scope IDs, one per
// line. For each scope ID in its command file, the scope sends a subsearch to that scope.
// Once a scope's subsearch completes, it pushes a single result with the scope's ID as the
// category ID. This allows us to set up various callgraphs by writing to the various command files.

TEST_F(ThrowingScopeTest, no_error)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(1, r.size());
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
    EXPECT_EQ(0, r.size());
}

TEST_F(ThrowingScopeTest, throw_from_run)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from run", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("QueryBase::run(): unity::ResourceException: exception from run", receiver->completion_msg());
    auto r = receiver->results();
    EXPECT_EQ(0, r.size());
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
    EXPECT_EQ(0, r.size());

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
    EXPECT_EQ(1, r.size());

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
    EXPECT_EQ(1, r.size());

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
    EXPECT_EQ(1, r.size());

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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;

    // Set the "TEST_DESKTOP_FILES_DIR" env var before forking as not to create desktop files in ~/.local
    putenv(const_cast<char*>("TEST_DESKTOP_FILES_DIR=" TEST_RUNTIME_PATH));

    auto rpid = fork();
    if (rpid == 0)
    {
        const char* const args[] = {"scoperegistry [Exception_test]", TEST_RUNTIME_FILE, nullptr};
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
