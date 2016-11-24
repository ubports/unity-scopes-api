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
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/SearchMetadata.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

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
        location_data_ = result["location_data"].get_bool();
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
        cond_.wait(lock, [this]
                   {
                       return this->query_complete_;
                   });
        query_complete_ = false;
    }

    bool location_data()
    {
        lock_guard<mutex> lock(mutex_);
        return location_data_;
    }

private:
    bool location_data_;
    bool query_complete_;
    mutable mutex mutex_;
    condition_variable cond_;
};

class StripLocationTest : public ::testing::Test
{
public:
    StripLocationTest()
    {
        runtime_ = Runtime::create(TEST_RUNTIME_FILE);
        auto reg = runtime_->registry();
        auto meta = reg->get_metadata("Aggregator");
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

char const* leaf_settings = TEST_RUNTIME_PATH "/config/Leaf/settings.ini";

TEST_F(StripLocationTest, basic)
{
    // Make sure we start out without any settings file.
    ::unlink(leaf_settings);

    {
        // No location data provided, so nothing to strip
        auto receiver = make_shared<Receiver>();
        scope()->search("unused", SearchMetadata("unused", "unused"), receiver);
        receiver->wait_until_finished();
        EXPECT_FALSE(receiver->location_data());
    }

    {
        // Location data provided. Default is true, so the leaf scope must receive it.
        auto receiver = make_shared<Receiver>();
        SearchMetadata smd("unused", "unused");
        Location loc(10, 20);
        smd.set_location(loc);
        scope()->search("unused", smd, receiver);
        receiver->wait_until_finished();
        EXPECT_TRUE(receiver->location_data());
    }

    {
        // Create settings file for leaf scope that allows it to get location data
        string cmd = string("cat >") + leaf_settings + " << EOF\n[General]\ninternal.location = true\nEOF";
        system(cmd.c_str());
    }

    {
        // No location data provided, so nothing to strip
        auto receiver = make_shared<Receiver>();
        scope()->search("unused", SearchMetadata("unused", "unused"), receiver);
        receiver->wait_until_finished();
        EXPECT_FALSE(receiver->location_data());
    }

    {
        // Location data provided, and settings file gives permission, so the leaf scope must receive it.
        auto receiver = make_shared<Receiver>();
        SearchMetadata smd("unused", "unused");
        Location loc(10, 20);
        smd.set_location(loc);
        scope()->search("unused", smd, receiver);
        receiver->wait_until_finished();
        EXPECT_TRUE(receiver->location_data());
    }

    {
        // Same test again, but this time the answer comes from the cache.
        auto receiver = make_shared<Receiver>();
        SearchMetadata smd("unused", "unused");
        Location loc(10, 20);
        smd.set_location(loc);
        scope()->search("unused", smd, receiver);
        receiver->wait_until_finished();
        EXPECT_TRUE(receiver->location_data());
    }

    {
        // Remove leaf scope permission. We need a sleep here because
        // time_t has a resolution of 1 second.
        sleep(2);
        string cmd;
        cmd = string("cat >") + leaf_settings + " << EOF\n[General]\ninternal.location = false\nEOF";
        system(cmd.c_str());
    }

    {
        // Location data provided, and settings file denies permission, so the leaf scope must not receive it.
        auto receiver = make_shared<Receiver>();
        SearchMetadata smd("unused", "unused");
        Location loc(10, 20);
        smd.set_location(loc);
        scope()->search("unused", smd, receiver);
        receiver->wait_until_finished();
        EXPECT_FALSE(receiver->location_data());
    }

    {
        // Same test again, but this time the answer comes from the cache.
        auto receiver = make_shared<Receiver>();
        SearchMetadata smd("unused", "unused");
        Location loc(10, 20);
        smd.set_location(loc);
        scope()->search("unused", smd, receiver);
        receiver->wait_until_finished();
        EXPECT_FALSE(receiver->location_data());
    }

    {
        // Unlink the settings file, so the default takes effect again.
        ::unlink(leaf_settings);

        auto receiver = make_shared<Receiver>();
        SearchMetadata smd("unused", "unused");
        Location loc(10, 20);
        smd.set_location(loc);
        scope()->search("unused", smd, receiver);
        receiver->wait_until_finished();
        EXPECT_TRUE(receiver->location_data());
    }
}

#pragma GCC diagnostic pop

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;

    // Set the "TEST_DESKTOP_FILES_DIR" env var before forking as not to create desktop files in ~/.local
    putenv(const_cast<char*>("TEST_DESKTOP_FILES_DIR=" TEST_RUNTIME_PATH));

    auto rpid = fork();
    if (rpid == 0)
    {
        const char* const args[] = {"scoperegistry [StripLocation_test]", TEST_RUNTIME_FILE, nullptr};
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
