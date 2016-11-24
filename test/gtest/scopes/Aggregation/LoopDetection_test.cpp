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
#include <unity/scopes/SearchMetadata.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include "AggTestScope.h"

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class CountReceiver : public SearchListenerBase
{
public:
    CountReceiver()
        : query_complete_(false)
    {
    }

    virtual void push(CategorisedResult result) override
    {
        string cat = result.category()->id();
        string title = result.title();

        lock_guard<mutex> lock(mutex_);
        auto it = results_.find(cat);
        if (it != results_.end())
        {
            it->second.push_back(title);
        }
        else
        {
            results_.insert(make_pair(cat, vector<string>{ title }));
        }
        cerr << "inserted " << cat << ", " << title << endl;
    }

    virtual void finished(CompletionDetails const& details) override
    {
        EXPECT_EQ(CompletionDetails::OK, details.status());
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

    map<string, vector<string>> results() const
    {
        lock_guard<mutex> lock(mutex_);

        return results_;
    }

private:
    map<string, vector<string>> results_;
    bool query_complete_;
    mutable mutex mutex_;
    condition_variable cond_;
};

class LoopDetectionTest : public ::testing::Test
{
public:
    LoopDetectionTest()
    {
        runtime_ = Runtime::create(TEST_RUNTIME_FILE);
        auto reg = runtime_->registry();
        auto meta = reg->get_metadata("A");  // First scope to receive a search is always A
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

// Stop warnings about unused return value from system()

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

// The command file for each scope is called A.cmd, B.cmd, etc. It contains scope IDs, one per
// line. For each scope ID in its command file, the scope sends a subsearch to that scope.
// Once a scope's subsearch completes, it pushes a single result with the scope's ID as the
// category ID. This allows us to set up various callgraphs by writing to the various command files.

TEST_F(LoopDetectionTest, no_error)
{
    system("cat >A.cmd << EOF\nB\nC\nEOF");
    system(">B.cmd");
    system(">C.cmd");
    auto receiver = make_shared<CountReceiver>();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    auto r = receiver->results();
    EXPECT_EQ(3u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());
    EXPECT_TRUE(r.find("C") != r.end());
}

TEST_F(LoopDetectionTest, diamond)
{
    system("cat >A.cmd << EOF\nB\nC\nEOF");
    system("cat >B.cmd << EOF\nD\nEOF");
    system("cat >C.cmd << EOF\nD\nEOF");
    system(">D.cmd");
    auto receiver = make_shared<CountReceiver>();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    auto r = receiver->results();
    EXPECT_EQ(4u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());
    EXPECT_TRUE(r.find("C") != r.end());
    EXPECT_TRUE(r.find("D") != r.end());
    EXPECT_EQ(2u, r.find("D")->second.size());  // B, and C each forwarded to D, so there must be 2 results.
}

TEST_F(LoopDetectionTest, immediate_loop)
{
    system("cat >A.cmd << EOF\nA\nEOF");
    auto receiver = make_shared<CountReceiver>();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    auto r = receiver->results();
    EXPECT_EQ(1u, r.size());
    EXPECT_EQ(3u, r.find("A")->second.size());  // "A" has sent the query twice to itself, second attempt stops loop
}

TEST_F(LoopDetectionTest, intermediate_loop)
{
    system("cat >A.cmd << EOF\nB\nEOF");
    system("cat >B.cmd << EOF\nC\nEOF");
    system("cat >C.cmd << EOF\nA\nEOF");
    auto receiver = make_shared<CountReceiver>();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    auto r = receiver->results();
    EXPECT_EQ(3u, r.size());
    EXPECT_EQ(2u, r.find("A")->second.size());  // "A" got results from B and C
    EXPECT_EQ(2u, r.find("B")->second.size());  // "B" got results from C and A
    EXPECT_EQ(1u, r.find("C")->second.size());  // "C" got result from A
}

TEST_F(LoopDetectionTest, repeated_search_on_leaf)
{
    system("cat >A.cmd << EOF\nB\nC\nD\nEOF");
    system("cat >B.cmd << EOF\nD\nD\nEOF");
    system("cat >C.cmd << EOF\nD\nEOF");
    system(">D.cmd");
    auto receiver = make_shared<CountReceiver>();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    auto r = receiver->results();
    EXPECT_EQ(4u, r.size());
    EXPECT_EQ(1u, r.find("A")->second.size());
    EXPECT_EQ(1u, r.find("B")->second.size());
    EXPECT_EQ(1u, r.find("C")->second.size());
    EXPECT_EQ(3u, r.find("D")->second.size());
}

TEST_F(LoopDetectionTest, repeated_search_on_aggregator)
{
    system("cat >A.cmd << EOF\nB\nEOF");
    system("cat >B.cmd << EOF\nC\nC\nEOF");
    system(">C.cmd << EOF\nD\nEOF");
    system(">D.cmd");
    auto receiver = make_shared<CountReceiver>();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    auto r = receiver->results();
    EXPECT_EQ(3u, r.size());
    EXPECT_EQ(1u, r.find("A")->second.size());
    EXPECT_EQ(1u, r.find("B")->second.size());
    EXPECT_EQ(1u, r.find("C")->second.size());
}

#pragma GCC diagnostic pop

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;

    // Set the "TEST_DESKTOP_FILES_DIR" env var before forking as not to create desktop files in ~/.local
    putenv(const_cast<char*>("TEST_DESKTOP_FILES_DIR=" TEST_RUNTIME_PATH));

    auto rpid = fork();
    if (rpid == 0)
    {
        const char* const args[] = {"scoperegistry [LoopDetection_test]", TEST_RUNTIME_FILE, nullptr};
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
