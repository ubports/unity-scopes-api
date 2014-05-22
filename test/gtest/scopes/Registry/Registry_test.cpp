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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/Runtime.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/CategorisedResult.h>
#include <gtest/gtest.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <signal.h>
#include <thread>
#include <unistd.h>

using namespace unity::scopes;

class Receiver : public SearchListenerBase
{
public:
    Receiver()
        : done_(false)
        , finished_ok_(false)
    {
    }

    virtual void push(CategorisedResult /* result */) override
    {
    }

    virtual void finished(ListenerBase::Reason reason, std::string const& error_message ) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        EXPECT_EQ(Finished, reason);
        EXPECT_EQ("", error_message);
        finished_ok_ = reason == Finished;
        done_ = true;
        cond_.notify_all();
    }

    bool wait_until_finished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        auto expiry_time = now + std::chrono::seconds(5);
        EXPECT_TRUE(cond_.wait_until(lock, expiry_time, [this]{ return done_; })) << "finished message not delivered";
        return finished_ok_;
    }

private:
    bool done_;
    bool finished_ok_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

TEST(Registry, metadata)
{
    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    auto meta = r->get_metadata("testscopeA");
    EXPECT_EQ("testscopeA", meta.scope_id());
    EXPECT_EQ("Canonical Ltd.", meta.author());
    EXPECT_EQ("scope-A.DisplayName", meta.display_name());
    EXPECT_EQ("scope-A.Description", meta.description());
    EXPECT_EQ("/foo/scope-A.Art", meta.art());
    EXPECT_EQ("/foo/scope-A.Icon", meta.icon());
    EXPECT_EQ("scope-A.HotKey", meta.hot_key());
    EXPECT_EQ("scope-A.SearchHint", meta.search_hint());
    EXPECT_EQ(TEST_RUNTIME_PATH "/scopes/testscopeA", meta.scope_directory());

    const char *bart = TEST_RUNTIME_PATH "/scopes/testscopeB/data/scope-B.Art";
    const char *bicon = TEST_RUNTIME_PATH "/scopes/testscopeB/data/scope-B.Icon";

    meta = r->get_metadata("testscopeB");
    EXPECT_EQ("testscopeB", meta.scope_id());
    EXPECT_EQ("Canonical Ltd.", meta.author());
    EXPECT_EQ("scope-B.DisplayName", meta.display_name());
    EXPECT_EQ("scope-B.Description", meta.description());
    EXPECT_EQ(bart, meta.art());
    EXPECT_EQ(bicon, meta.icon());
    EXPECT_EQ("scope-B.HotKey", meta.hot_key());
    EXPECT_EQ("scope-B.SearchHint", meta.search_hint());
    EXPECT_EQ(TEST_RUNTIME_PATH "/scopes/testscopeB", meta.scope_directory());

    auto sp = meta.proxy();

    auto receiver = std::make_shared<Receiver>();
    SearchListenerBase::SPtr reply(receiver);
    SearchMetadata metadata("C", "desktop");

    // search would fail if testscopeB can't be executed
    auto ctrl = sp->search("foo", metadata, reply);
    EXPECT_TRUE(receiver->wait_until_finished());
}

TEST(Registry, update_notify)
{
    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    // Configure registry update callback
    bool update_received_ = false;
    std::mutex mutex_;
    std::condition_variable cond_;
    r->set_update_callback([&update_received_, &mutex_, &cond_]
    {
        std::lock_guard<std::mutex> lock(mutex_);
        update_received_ = true;
        cond_.notify_one();
    });
    auto wait_for_update = [&update_received_, &mutex_, &cond_]
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [&update_received_] { return update_received_; });
        update_received_ = false;
    };

    // First check that we have 2 scopes registered
    MetadataMap list = r->list();
    EXPECT_EQ(2, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));

    // Move testscopeC into the scopes folder
    ASSERT_EQ(0, system("mv -fv '" TEST_RUNTIME_PATH "/other_scopes/testscopeC' '" TEST_RUNTIME_PATH "/scopes'"));
    wait_for_update();

    // Now check that we have 3 scopes registered
    list = r->list();
    EXPECT_EQ(3, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeC"));

    // Make a symlink to testscopeD in the scopes folder
    ASSERT_EQ(0, system("ln -sfv '" TEST_RUNTIME_PATH "/other_scopes/testscopeD' '" TEST_RUNTIME_PATH "/scopes/testscopeD'"));
    wait_for_update();

    // Now check that we have 4 scopes registered
    list = r->list();
    EXPECT_EQ(4, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeC"));
    EXPECT_NE(list.end(), list.find("testscopeD"));

    // Move testscopeC back into the other_scopes folder
    ASSERT_EQ(0, system("mv -fv '" TEST_RUNTIME_PATH "/scopes/testscopeC' '" TEST_RUNTIME_PATH "/other_scopes'"));
    wait_for_update();

    // Now check that we have 3 scopes registered again
    list = r->list();
    EXPECT_EQ(3, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeD"));

    // Remove symlink to testscopeD from the scopes folder
    ASSERT_EQ(0, system("rm -fv '" TEST_RUNTIME_PATH "/scopes/testscopeD'"));
    wait_for_update();

    // Now check that we are back to having 2 scopes registered
    list = r->list();
    EXPECT_EQ(2, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));

    // Make a folder in scopes that does not represent a scope id
    ASSERT_EQ(0, system("mkdir -pv '" TEST_RUNTIME_PATH "/scopes/testfolder'"));
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Check that no scopes were registered
    list = r->list();
    EXPECT_EQ(2, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));

    // Make a symlink to testscopeC.ini in testfolder
    ASSERT_EQ(0, system("ln -sfv '" TEST_RUNTIME_PATH "/other_scopes/testscopeC/testscopeC.ini' '" TEST_RUNTIME_PATH "/scopes/testfolder/testscopeC.ini'"));
    wait_for_update();

    // Now check that we have 3 scopes registered
    list = r->list();
    EXPECT_EQ(3, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeC"));

    // Remove testfolder
    ASSERT_EQ(0, system("rm -rfv '" TEST_RUNTIME_PATH "/scopes/testfolder'"));
    wait_for_update();

    // Now check that we are back to having 2 scopes registered
    list = r->list();
    EXPECT_EQ(2, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    auto rpid = fork();
    if (rpid == 0)
    {
        const char* const args[] = {"scoperegistry [Registry test]", TEST_RUNTIME_FILE, nullptr};
        if (execv(TEST_REGISTRY_PATH "/scoperegistry", const_cast<char* const*>(args)) < 0)
        {
            perror("Error starting scoperegistry:");
        }
        return 1;
    }
    else if (rpid > 0)
    {
        auto rc = RUN_ALL_TESTS();
        kill(rpid, SIGTERM);
        return rc;
    }
    else
    {
        perror("Failed to fork:");
    }
    return 1;
}
