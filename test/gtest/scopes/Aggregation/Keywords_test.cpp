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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/testing/ScopeMetadataBuilder.h>

#include <boost/filesystem/operations.hpp>

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
        bool is_aggregated = result["is_aggregated"].get_bool();
        string aggregated_keywords = result["aggregated_keywords"].get_string();

        lock_guard<mutex> lock(mutex_);
        results_[cat] = make_pair(is_aggregated, aggregated_keywords);
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

    map<string, pair<bool, string>> results() const
    {
        lock_guard<mutex> lock(mutex_);

        return results_;
    }

    void reset()
    {
        lock_guard<mutex> lock(mutex_);

        results_.clear();
        query_complete_ = false;
    }

private:
    map<string, pair<bool, string>> results_;
    bool query_complete_;
    mutable mutex mutex_;
    condition_variable cond_;
};

class KeywordsTest : public ::testing::Test
{
public:
    KeywordsTest()
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

    ScopeMetadata test_metadata()
    {
        unity::scopes::testing::ScopeMetadataBuilder builder;
        builder.scope_id(" ").proxy(scope_).display_name(" ").description(" ").author(" ");
        return builder();
    }

private:
    Runtime::UPtr runtime_;
    ScopeProxy scope_;
};

class RaiiConfigDir
{
public:
    RaiiConfigDir()
    {
        // Create an empty config directory for scope A
        boost::system::error_code ec;
        boost::filesystem::remove_all(TEST_RUNTIME_PATH "/A", ec);
        boost::filesystem::create_directory(TEST_RUNTIME_PATH "/A", ec);
    }

    ~RaiiConfigDir()
    {
        // Remove the config directory for scope A
        boost::system::error_code ec;
        boost::filesystem::remove_all(TEST_RUNTIME_PATH "/A", ec);
    }
};

// Stop warnings about unused return value from system()

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"

// The command file for each scope is called A.cmd, B.cmd, etc. It contains scope IDs, one per
// line. For each scope ID in its command file, the scope sends a subsearch to that scope.
// Once a scope's subsearch completes, it pushes a single result with the scope's ID as the
// category ID. This allows us to set up various callgraphs by writing to the various command files.

TEST_F(KeywordsTest, subsearch_disabled_child)
{
    // Create an empty config directory for scope A
    RaiiConfigDir config_dir;

    system("cat >A.cmd << EOF\nB\nC\nD\nEOF");
    system(">B.cmd");
    system(">C.cmd");
    system(">D.cmd");
    auto receiver = make_shared<CountReceiver>();

    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    // Should see all scopes' results here as all children are enabled by default
    auto r = receiver->results();
    EXPECT_EQ(4u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());
    EXPECT_TRUE(r.find("C") != r.end());
    EXPECT_TRUE(r.find("D") != r.end());

    // Disable child scope "C"
    {
        ChildScopeList list;
        list.emplace_back(ChildScope{"B", test_metadata(), true});
        list.emplace_back(ChildScope{"C", test_metadata(), false});
        list.emplace_back(ChildScope{"D", test_metadata(), true});
        scope()->set_child_scopes(list);
    }

    receiver->reset();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    // Should not see scope C's results here as it has been disabled
    r = receiver->results();
    EXPECT_EQ(3u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());
    EXPECT_TRUE(r.find("D") != r.end());

    // Disable child scope "D"
    {
        ChildScopeList list;
        list.emplace_back(ChildScope{"B", test_metadata(), true});
        list.emplace_back(ChildScope{"C", test_metadata(), false});
        list.emplace_back(ChildScope{"D", test_metadata(), false});
        scope()->set_child_scopes(list);
    }

    receiver->reset();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    // Should not see scope C and D's results here as they are disabled
    r = receiver->results();
    EXPECT_EQ(2u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());

    // Re-enable child scope "C"
    {
        ChildScopeList list;
        list.emplace_back(ChildScope{"B", test_metadata(), true});
        list.emplace_back(ChildScope{"C", test_metadata(), true});
        list.emplace_back(ChildScope{"D", test_metadata(), false});
        scope()->set_child_scopes(list);
    }

    receiver->reset();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    // Should not see scope D's results here as it is still disabled
    r = receiver->results();
    EXPECT_EQ(3u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());
    EXPECT_TRUE(r.find("C") != r.end());

    // Re-enable all child scopes
    {
        ChildScopeList list;
        list.emplace_back(ChildScope{"B", test_metadata(), true});
        list.emplace_back(ChildScope{"C", test_metadata(), true});
        list.emplace_back(ChildScope{"D", test_metadata(), true});
        scope()->set_child_scopes(list);
    }

    receiver->reset();
    scope()->search("A", SearchMetadata("unused", "unused"), receiver);
    receiver->wait_until_finished();

    // Should see all scopes' results here as all children are enabled again
    r = receiver->results();
    EXPECT_EQ(4u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());
    EXPECT_TRUE(r.find("C") != r.end());
    EXPECT_TRUE(r.find("D") != r.end());
}

TEST_F(KeywordsTest, subsearch_with_keywords)
{
    // Create an empty config directory for scope A
    RaiiConfigDir config_dir;

    system("cat >A.cmd << EOF\nB\nC\nD\nEOF");
    system(">B.cmd");
    system(">C.cmd");
    system(">D.cmd");
    auto receiver = make_shared<CountReceiver>();

    // Provide test keywords via search metadata so that the test scope can insert them
    // accordingly during find_child_scopes()
    SearchMetadata meta("unused", "unused");
    meta["B"] = VariantArray{Variant("Bkw1"), Variant("Bkw2")};
    meta["C"] = VariantArray{Variant("Ckw1")};
    meta["D"] = VariantArray{Variant("Dkw1"), Variant("Dkw2"), Variant("Dkw3"), Variant("Dkw4")};

    scope()->search("A", meta, receiver);
    receiver->wait_until_finished();

    // Should see all scopes' results here as all children are enabled by default
    auto r = receiver->results();
    EXPECT_EQ(4u, r.size());
    EXPECT_TRUE(r.find("A") != r.end());
    EXPECT_TRUE(r.find("B") != r.end());
    EXPECT_TRUE(r.find("C") != r.end());
    EXPECT_TRUE(r.find("D") != r.end());

    // Check is_aggregated states
    EXPECT_FALSE(r.at("A").first);
    EXPECT_TRUE(r.at("B").first);
    EXPECT_TRUE(r.at("C").first);
    EXPECT_TRUE(r.at("D").first);

    // Check aggregated_keywords
    EXPECT_EQ("", r.at("A").second);
    EXPECT_EQ("Bkw1, Bkw2", r.at("B").second);
    EXPECT_EQ("Ckw1", r.at("C").second);
    EXPECT_EQ("Dkw1, Dkw2, Dkw3, Dkw4", r.at("D").second);
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
        const char* const args[] = {"scoperegistry [Keywords_test]", TEST_RUNTIME_FILE, nullptr};
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
