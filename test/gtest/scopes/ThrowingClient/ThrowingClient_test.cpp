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

#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/Runtime.h>

#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <condition_variable>

#include "EchoScope.h"

using namespace std;
using namespace unity;
using namespace unity::scopes;

class Receiver : public SearchListenerBase
{
public:
    Receiver(string const& command = "")
        : query_complete_(false)
        , command_(command)
    {
    }

    virtual void push(Department::SCPtr const& parent) override
    {
        auto child_list = parent->subdepartments();
        assert(child_list.size() == 1);
        if (child_list.front()->id() == "throw from push department")
        {
            throw ResourceException("exception from push department");
        }
    }

    virtual void push(CategorisedResult result) override
    {
        if (result.title() == "throw from push result")
        {
            throw ResourceException("exception from push result");
        }
        lock_guard<mutex> lock(mutex_);
        results_.push_back(move(result));
    }

    virtual void push(experimental::Annotation annotation) override
    {
        if (annotation.label() == "throw from push annotation")
        {
            throw ResourceException("exception from push annotation");
        }
    }

    virtual void push(Category::SCPtr const& category) override
    {
        if (category->id() == "throw from push category")
        {
            throw ResourceException("exception from push category");
        }
    }

    virtual void push(Filters const& filters, FilterState const& /* filter_state */) override
    {
        assert(filters.size() == 1);
        if (filters.front()->id() == "throw from push filter")
        {
            throw ResourceException("exception from push filter");
        }
    }

    virtual void finished(CompletionDetails const& details) override
    {
        lock_guard<mutex> lock(mutex_);

        completion_status_ = details.status();
        completion_msg_ = details.message();
        query_complete_ = true;
        cond_.notify_one();

        if (command_ == "throw from finished")
        {
            throw ResourceException("exception from finished");
        }
    }

    virtual void info(OperationInfo const& /* op_info */) override
    {
        lock_guard<mutex> lock(mutex_);

        if (command_ == "throw from info")
        {
            throw ResourceException("exception from info");
        }
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
    string command_;
    mutex mutex_;
    condition_variable cond_;
};

class PreviewListener : public PreviewListenerBase
{
public:
    PreviewListener(string const& command = "")
        : preview_complete_(false)
        , command_(command)
    {
    }

    void finished(const unity::scopes::CompletionDetails& details) override
    {
        lock_guard<mutex> lock(mutex_);

        completion_status_ = details.status();
        completion_msg_ = details.message();
        preview_complete_ = true;
        cond_.notify_one();

        if (command_ == "throw from preview finished")
        {
            throw ResourceException("exception from preview finished");
        }
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
        if (command_ == "throw from preview push layout")
        {
            throw ResourceException("exception from preview push layout");
        }
    }

    void push(PreviewWidgetList const&) override
    {
        if (command_ == "throw from preview push widget")
        {
            throw ResourceException("exception from preview push widget");
        }
    }

    void push(std::string const&, Variant const&) override
    {
        if (command_ == "throw from preview push attribute")
        {
            throw ResourceException("exception from preview push attribute");
        }
    }

private:
    bool preview_complete_;
    string completion_msg_;
    CompletionDetails::CompletionStatus completion_status_;
    string command_;
    mutex mutex_;
    condition_variable cond_;
};

class ActivationListener : public ActivationListenerBase
{
public:
    ActivationListener(string const& command = "")
        : activate_complete_(false)
        , command_(command)
    {
    }

    void activated(ActivationResponse const& response) override
    {
        lock_guard<mutex> lock(mutex_);

        act_status_ = response.status();
        if (command_ == "throw from activation activate")
        {
            throw ResourceException("exception from activation activate");
        }
    }

    void finished(const unity::scopes::CompletionDetails& details) override
    {
        lock_guard<mutex> lock(mutex_);

        completion_status_ = details.status();
        completion_msg_ = details.message();
        activate_complete_ = true;
        cond_.notify_one();

        if (command_ == "throw from activation finished")
        {
            throw ResourceException("exception from activation finished");
        }
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
    string command_;
    mutex mutex_;
    condition_variable cond_;
};

class EchoScopeTest : public ::testing::Test
{
public:
    EchoScopeTest()
    {
        runtime_ = Runtime::create(TEST_RUNTIME_FILE);
        auto reg = runtime_->registry();
        auto meta = reg->get_metadata("EchoScope");
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

TEST_F(EchoScopeTest, no_error)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ("success", r[0].title());
}

TEST_F(EchoScopeTest, throw_from_push_result)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from push result", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from push result",
              receiver->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_push_department)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from push department", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from push department",
              receiver->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_push_category)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from push category", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from push category",
              receiver->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_push_filter)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from push filter", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from push filter",
              receiver->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_push_annotation)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("throw from push annotation", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, receiver->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from push annotation",
              receiver->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_finished)
{
    auto receiver = make_shared<Receiver>("throw from finished");
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_info)
{
    auto receiver = make_shared<Receiver>("throw from info");
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
}

TEST_F(EchoScopeTest, preview_no_error)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto plistener = make_shared<PreviewListener>();
    scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, plistener->completion_status());
    EXPECT_EQ("", plistener->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_preview_push_layout)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto plistener = make_shared<PreviewListener>("throw from preview push layout");
    scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, plistener->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from preview push layout",
              plistener->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_preview_push_widget)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto plistener = make_shared<PreviewListener>("throw from preview push widget");
    scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, plistener->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from preview push widget",
              plistener->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_preview_push_attribute)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto plistener = make_shared<PreviewListener>("throw from preview push attribute");
    scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, plistener->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from preview push attribute",
              plistener->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_preview_finished)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto plistener = make_shared<PreviewListener>("throw from preview finished");
    scope()->preview(r[0], ActionMetadata("unused", "unused"), plistener);
    plistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, plistener->completion_status());
    EXPECT_EQ("", plistener->completion_msg());
}

TEST_F(EchoScopeTest, activation_no_error)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto alistener = make_shared<ActivationListener>();
    scope()->activate(r[0], ActionMetadata("unused", "unused"), alistener);
    alistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, alistener->completion_status());
    EXPECT_EQ("", alistener->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_activation_activate)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto alistener = make_shared<ActivationListener>("throw from activation activate");
    scope()->activate(r[0], ActionMetadata("unused", "unused"), alistener);
    alistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::Error, alistener->completion_status());
    EXPECT_EQ("ReplyObject::push(VariantMap): unity::ResourceException: exception from activation activate",
              alistener->completion_msg());
}

TEST_F(EchoScopeTest, throw_from_activation_finished)
{
    auto receiver = make_shared<Receiver>();
    scope()->search("success", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, receiver->completion_status());
    EXPECT_EQ("", receiver->completion_msg());
    auto r = receiver->results();
    ASSERT_EQ(1u, r.size());
    EXPECT_EQ(r[0].title(), "success");

    auto alistener = make_shared<ActivationListener>("throw from activation finished");
    scope()->activate(r[0], ActionMetadata("unused", "unused"), alistener);
    alistener->wait_until_finished();

    EXPECT_EQ(CompletionDetails::CompletionStatus::OK, alistener->completion_status());
    EXPECT_EQ("", alistener->completion_msg());
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
        const char* const args[] = {"scoperegistry [EchoScope_test]", TEST_RUNTIME_FILE, nullptr};
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
