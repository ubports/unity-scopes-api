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

#include <unity/scopes/Runtime.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/SearchMetadata.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <chrono>
#include <condition_variable>

using namespace std;
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

    virtual void finished(CompletionDetails const& details) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        EXPECT_EQ(CompletionDetails::OK, details.status());
        EXPECT_EQ("", details.message());
        finished_ok_ = details.status() == CompletionDetails::OK;
        done_ = true;
        cond_.notify_all();
    }

    bool wait_until_finished()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto now = std::chrono::steady_clock::now();
        auto expiry_time = now + std::chrono::seconds(5);
        EXPECT_TRUE(cond_.wait_until(lock, expiry_time, [this]{ return done_; })) << "finished message not delivered";
        done_ = false;
        return finished_ok_;
    }

private:
    bool done_;
    bool finished_ok_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

TEST(StandAloneScope, reachable)
{
    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);  // No registry configured.

    try
    {
        rt->registry();
        FAIL();
    }
    catch (ConfigException const& e)
    {
        ASSERT_STREQ("unity::scopes::ConfigException: registry(): no registry configured", e.what());
    }

    string stringified_proxy = "ipc:///tmp/priv/SimpleScope#SimpleScope!t=1000!c=Scope";
    ObjectProxy p = rt->string_to_proxy(stringified_proxy);
    ASSERT_TRUE(bool(p));

    ScopeProxy sp = dynamic_pointer_cast<Scope>(p);
    ASSERT_TRUE(bool(sp));

    // We must be able to reach the scope even though the registry is not running.
    shared_ptr<Receiver> reply(new Receiver);
    auto ctrl = sp->search("", SearchMetadata("", ""), reply);
    EXPECT_TRUE(reply->wait_until_finished());
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 1;

    // Set the "TEST_DESKTOP_FILES_DIR" env var before forking as not to create desktop files in ~/.local
    putenv(const_cast<char*>("TEST_DESKTOP_FILES_DIR=" TEST_RUNTIME_PATH));

    auto rpid = fork();
    if (rpid == 0)
    {
        // Start a scoperunner without any registry config.
        const char* const args[] = {"scoperunner [StandAloneScope test]", TEST_RUNTIME_FILE, "SimpleScope.ini", nullptr};
        if (execv(TEST_SCOPERUNNER_PATH "/scoperunner", const_cast<char* const*>(args)) < 0)
        {
            perror("Error starting scoperunner:");
        }
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
