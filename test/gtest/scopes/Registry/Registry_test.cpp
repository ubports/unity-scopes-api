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

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/Runtime.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/SearchMetadata.h>

#include <boost/filesystem/operations.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <mutex>
#include <signal.h>
#include <thread>
#include <unistd.h>

using namespace boost;
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
    EXPECT_EQ("file://scope-A.Icon", meta.icon());
    EXPECT_EQ("scope-A.HotKey", meta.hot_key());
    EXPECT_FALSE(meta.invisible());
    EXPECT_EQ("scope-A.SearchHint", meta.search_hint());
    EXPECT_EQ(TEST_RUNTIME_PATH "/scopes/testscopeA", meta.scope_directory());
    auto defs = meta.settings_definitions();
    ASSERT_EQ(2u, defs.size());
    EXPECT_EQ("locationSetting", defs[0].get_dict()["id"].get_string());
    EXPECT_EQ("Location", defs[0].get_dict()["displayName"].get_string());
    EXPECT_EQ("string", defs[0].get_dict()["type"].get_string());
    EXPECT_EQ("London", defs[0].get_dict()["defaultValue"].get_string());
    EXPECT_EQ("internal.location", defs[1].get_dict()["id"].get_string());
    EXPECT_EQ("Enable location data", defs[1].get_dict()["displayName"].get_string());
    EXPECT_EQ("boolean", defs[1].get_dict()["type"].get_string());
    EXPECT_EQ(true, defs[1].get_dict()["defaultValue"].get_bool());
    EXPECT_TRUE(meta.location_data_needed());
    auto children = meta.child_scope_ids();
    EXPECT_EQ(2u, children.size());
    EXPECT_EQ("com.foo.bar", children[0]);
    EXPECT_EQ("com.foo.baz", children[1]);
    EXPECT_EQ(1, meta.version());
    auto keywords = meta.keywords();
    EXPECT_EQ(3u, keywords.size());
    EXPECT_NE(keywords.end(), keywords.find("music"));
    EXPECT_NE(keywords.end(), keywords.find("news"));
    EXPECT_NE(keywords.end(), keywords.find("foo"));
    EXPECT_FALSE(meta.is_aggregator());

    auto attrs = meta.appearance_attributes();
    EXPECT_EQ("fg_color", attrs["foreground-color"].get_string());
    EXPECT_EQ("bg_color", attrs["background-color"].get_string());
    EXPECT_TRUE(attrs["shape-images"].get_bool());
    EXPECT_EQ("preview_button_color", attrs["preview-button-color"].get_string());
    EXPECT_EQ("overlay_color", attrs["logo-overlay-color"].get_string());
    auto page_hdr = attrs["page-header"].get_dict();
    EXPECT_EQ(TEST_SCOPE_A_PATH "/some_url", page_hdr["logo"].get_string());
    EXPECT_EQ("fg_color", page_hdr["foreground-color"].get_string());
    EXPECT_EQ("div_color", page_hdr["divider-color"].get_string());
    EXPECT_EQ("http://nav_background", page_hdr["navigation-background"].get_string());

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
    defs = meta.settings_definitions();
    EXPECT_EQ(0u, defs.size());
    EXPECT_FALSE(meta.location_data_needed());
    EXPECT_EQ(0u, meta.child_scope_ids().size());
    EXPECT_EQ(0u, meta.keywords().size());
    EXPECT_EQ(0, meta.version());
    EXPECT_FALSE(meta.is_aggregator());
}

auto const wait_for_update_time = std::chrono::milliseconds(5000);

TEST(Registry, scope_state_notify)
{
    bool updateA_received = false;
    bool updateB_received = false;
    bool testscopeA_state = false;
    bool testscopeB_state = false;
    std::mutex mutex;
    std::condition_variable cond;

    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    // Configure testscopeA scope_state_callback
    auto connA = r->set_scope_state_callback("testscopeA", [&updateA_received, &testscopeA_state, &mutex, &cond](bool is_running)
    {
        std::lock_guard<std::mutex> lock(mutex);
        updateA_received = true;
        testscopeA_state = is_running;
        cond.notify_one();
    });

    // Configure testscopeB scope_state_callback
    auto connB = r->set_scope_state_callback("testscopeB", [&updateB_received, &testscopeB_state, &mutex, &cond](bool is_running)
    {
        std::lock_guard<std::mutex> lock(mutex);
        updateB_received = true;
        testscopeB_state = is_running;
        cond.notify_one();
    });

    auto wait_for_stateA_update = [&updateA_received, &mutex, &cond]
    {
        // Wait for an update notification
        std::unique_lock<std::mutex> lock(mutex);
        bool success = cond.wait_for(lock, wait_for_update_time, [&updateA_received] { return updateA_received; });
        updateA_received = false;
        return success;
    };

    auto wait_for_stateB_update = [&updateB_received, &mutex, &cond]
    {
        // Wait for an update notification
        std::unique_lock<std::mutex> lock(mutex);
        bool success = cond.wait_for(lock, wait_for_update_time, [&updateB_received] { return updateB_received; });
        updateB_received = false;
        return success;
    };

    auto get_stateA = [&testscopeA_state, &mutex] { std::lock_guard<std::mutex> lock(mutex); return testscopeA_state; };
    auto get_stateB = [&testscopeB_state, &mutex] { std::lock_guard<std::mutex> lock(mutex); return testscopeB_state; };

    auto receiver_A = std::make_shared<Receiver>();
    SearchListenerBase::SPtr reply_A(receiver_A);
    SearchMetadata metadata("C", "desktop");

    auto meta = r->get_metadata("testscopeA");
    auto defs = meta.settings_definitions();
    EXPECT_EQ(2u, defs.size());
    auto sp = meta.proxy();

    // testscopeA should not be running at this point
    EXPECT_FALSE(r->is_scope_running("testscopeA"));

    // search would fail if testscopeA can't be executed
    auto ctrl = sp->search("foo", metadata, reply_A);
    EXPECT_TRUE(receiver_A->wait_until_finished());

    // testscopeA should now be running
    EXPECT_TRUE(wait_for_stateA_update());
    EXPECT_TRUE(get_stateA());
    EXPECT_TRUE(r->is_scope_running("testscopeA"));

    meta = r->get_metadata("testscopeB");
    defs = meta.settings_definitions();
    EXPECT_EQ(0u, defs.size());
    sp = meta.proxy();

    // testscopeB should not be running at this point
    EXPECT_FALSE(r->is_scope_running("testscopeB"));

    // search would fail if testscopeB can't be executed
    auto receiver_B = std::make_shared<Receiver>();
    SearchListenerBase::SPtr reply_B(receiver_B);
    ctrl = sp->search("foo", metadata, reply_B);
    EXPECT_TRUE(receiver_B->wait_until_finished());

    // testscopeB should now be running
    EXPECT_TRUE(wait_for_stateB_update());
    EXPECT_TRUE(get_stateB());
    EXPECT_TRUE(r->is_scope_running("testscopeB"));

    // check that the scope is still running after 1s (should only time out after 2s)
    std::this_thread::sleep_for(std::chrono::seconds{1});
    EXPECT_TRUE(get_stateB());
    EXPECT_TRUE(r->is_scope_running("testscopeB"));

    // check now that we get a callback when testscopeB terminates (timed out after 2s)
    EXPECT_TRUE(wait_for_stateB_update());
    EXPECT_FALSE(get_stateB());
    EXPECT_FALSE(r->is_scope_running("testscopeB"));
}

TEST(Registry, no_idle_timeout_in_debug_mode)
{
    bool update_received;
    std::mutex mutex;
    std::condition_variable cond;

    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    // Configure testscopeC scope_state_callback
    auto conn = r->set_scope_state_callback("testscopeC", [&update_received, &mutex, &cond](bool)
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = true;
        cond.notify_one();
    });

    auto reset = [&update_received, &mutex]
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = false;
    };

    auto wait_for_state_update = [&update_received, &mutex, &cond]
    {
        // Wait for an update notification
        std::unique_lock<std::mutex> lock(mutex);
        bool success = cond.wait_for(lock, wait_for_update_time, [&update_received] { return update_received; });
        update_received = false;
        return success;
    };

    system::error_code ec;

    // Copy testscopeC into the scopes folder
    filesystem::create_directory(TEST_RUNTIME_PATH "/scopes/testscopeC", ec);
    ASSERT_EQ("Success", ec.message());
    filesystem::copy(TEST_RUNTIME_PATH "/other_scopes/testscopeC/testscopeC.ini", TEST_RUNTIME_PATH "/scopes/testscopeC/testscopeC.ini", ec);
    ASSERT_EQ("Success", ec.message());
    filesystem::copy(TEST_RUNTIME_PATH "/other_scopes/testscopeC/libtestscopeC.so", TEST_RUNTIME_PATH "/scopes/testscopeC/libtestscopeC.so", ec);
    ASSERT_EQ("Success", ec.message());

    // Wait for the registry to realize that there is a new scope.
    auto const wait_time = std::chrono::seconds(5);
    auto start_time = std::chrono::system_clock::now();
    bool ok = false;
    do
    {
        try
        {
            // testscopeC should not be running at this point
            EXPECT_FALSE(r->is_scope_running("testscopeC"));
            ok = true;
        }
        catch (NotFoundException const&)
        {
        }
    }
    while (!ok && std::chrono::system_clock::now() - start_time < wait_time);
    EXPECT_TRUE(ok) << "new scope not recognized after " << wait_time.count() << " seconds";

    auto meta = r->get_metadata("testscopeC");
    auto sp = meta.proxy();

    // testscopeC should not be running at this point
    EXPECT_FALSE(r->is_scope_running("testscopeC"));

    reset();
    // search would fail if testscopeC can't be executed
    auto receiver = std::make_shared<Receiver>();
    SearchListenerBase::SPtr reply(receiver);
    auto ctrl = sp->search("foo", SearchMetadata("C", "desktop"), reply);
    EXPECT_TRUE(receiver->wait_until_finished());

    // testscopeC should now be running
    EXPECT_TRUE(wait_for_state_update());
    EXPECT_TRUE(r->is_scope_running("testscopeC"));

    // check that the scope is still running after 4s
    // (due to "DebugMode = true" and despite "IdleTimeout = 2")
    std::this_thread::sleep_for(std::chrono::seconds{4});
    EXPECT_TRUE(r->is_scope_running("testscopeC"));

    reset();
    // Remove testscopeC from the scopes folder
    filesystem::remove_all(TEST_RUNTIME_PATH "/scopes/testscopeC", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_state_update());
}

TEST(Registry, manually_started_scope)
{
    bool update_received = false;
    std::mutex mutex;
    std::condition_variable cond;

    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    // Configure testscopeC scope_state_callback
    auto conn = r->set_scope_state_callback("testscopeC", [&update_received, &mutex, &cond](bool)
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = true;
        cond.notify_one();
    });

    auto reset = [&update_received, &mutex]
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = false;
    };

    auto wait_for_state_update = [&update_received, &mutex, &cond]
    {
        // Wait for an update notification
        std::unique_lock<std::mutex> lock(mutex);
        bool success = cond.wait_for(lock, std::chrono::seconds(5), [&update_received] { return update_received; });
        update_received = false;
        return success;
    };

    system::error_code ec;

    reset();
    // Copy testscopeC into the scopes folder
    filesystem::create_directory(TEST_RUNTIME_PATH "/scopes/testscopeC", ec);
    ASSERT_EQ("Success", ec.message());
    filesystem::copy(TEST_RUNTIME_PATH "/other_scopes/testscopeC/testscopeC.ini", TEST_RUNTIME_PATH "/scopes/testscopeC/testscopeC.ini", ec);
    ASSERT_EQ("Success", ec.message());
    filesystem::copy(TEST_RUNTIME_PATH "/other_scopes/testscopeC/libtestscopeC.so", TEST_RUNTIME_PATH "/scopes/testscopeC/libtestscopeC.so", ec);
    ASSERT_EQ("Success", ec.message());

    // Wait for the registry to realize that there is a new scope.
    auto const wait_time = std::chrono::seconds(5);
    auto start_time = std::chrono::system_clock::now();
    bool ok = false;
    do
    {
        try
        {
            // testscopeC should not be running at this point
            EXPECT_FALSE(r->is_scope_running("testscopeC"));
            ok = true;
        }
        catch (NotFoundException const&)
        {
        }
    }
    while (!ok && std::chrono::system_clock::now() - start_time < wait_time);
    EXPECT_TRUE(ok) << "new scope not recognized after " << wait_time.count() << " seconds";

    reset();
    // start testscopeC manually
    auto scope_pid = fork();
    if (scope_pid == 0)
    {
        const char* const args[] = {"scoperunner [Registry test]", TEST_RUNTIME_FILE, TEST_RUNTIME_PATH "/scopes/testscopeC/testscopeC.ini", nullptr};

        if (execv(TEST_SCOPERUNNER_PATH "/scoperunner", const_cast<char* const*>(args)) < 0)
        {
            perror("Error starting scoperunner:");
        }
        exit(0);
    }

    // testscopeC should now be running
    EXPECT_TRUE(wait_for_state_update());
    EXPECT_TRUE(r->is_scope_running("testscopeC"));

    reset();
    // stop testscopeC manually
    kill(scope_pid, SIGTERM);
    int status;
    waitpid(scope_pid, &status, 0);

    // testscopeC should now be stopped
    EXPECT_TRUE(wait_for_state_update());
    EXPECT_FALSE(r->is_scope_running("testscopeC"));

    reset();
    // Remove testscopeC from the scopes folder
    filesystem::remove_all(TEST_RUNTIME_PATH "/scopes/testscopeC", ec);
    ASSERT_EQ("Success", ec.message());

    rt->destroy();
}

TEST(Registry, list_update_notify_before_click_folder_exists)
{
    bool update_received;
    std::mutex mutex;
    std::condition_variable cond;

    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    // Configure registry update callback
    auto conn = r->set_list_update_callback([&update_received, &mutex, &cond]
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = true;
        cond.notify_one();
    });

    auto reset = [&update_received, &mutex]
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = false;
    };

    auto wait_for_update = [&update_received, &mutex, &cond]
    {
        // Flush out update notifications
        std::unique_lock<std::mutex> lock(mutex);
        return cond.wait_for(lock, wait_for_update_time, [&update_received] { return update_received; });
    };

    system::error_code ec;

    // First check that we have 2 scopes registered
    MetadataMap list = r->list();
    EXPECT_EQ(2u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_EQ(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));

    std::cout << "Create click folder: " TEST_RUNTIME_PATH "/click" << std::endl;
    filesystem::create_directory(TEST_RUNTIME_PATH "/click", ec);
    ASSERT_EQ("Success", ec.message());

    reset();
    std::cout << "Make a symlink to testscopeC in the scopes folder" << std::endl;
    filesystem::create_symlink(TEST_RUNTIME_PATH "/other_scopes/testscopeC", TEST_RUNTIME_PATH "/click/testscopeC", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_update());

    // Now check that we have 3 scopes registered
    list = r->list();
    EXPECT_EQ(3u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));

    std::cout << "Remove click folder" << std::endl;
    filesystem::remove_all(TEST_RUNTIME_PATH "/click",ec);
    ASSERT_EQ("Success", ec.message());
}

TEST(Registry, list_update_notify)
{
    bool update_received;
    std::mutex mutex;
    std::condition_variable cond;

    Runtime::UPtr rt = Runtime::create(TEST_RUNTIME_FILE);
    RegistryProxy r = rt->registry();

    // Configure registry update callback
    auto conn = r->set_list_update_callback([&update_received, &mutex, &cond]
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = true;
        cond.notify_one();
    });

    auto reset = [&update_received, &mutex]
    {
        std::lock_guard<std::mutex> lock(mutex);
        update_received = false;
    };

    auto wait_for_update = [&update_received, &mutex, &cond]
    {
        std::unique_lock<std::mutex> lock(mutex);
        return cond.wait_for(lock, wait_for_update_time, [&update_received] { return update_received; });
    };

    system::error_code ec;

    // First check that we have 2 scopes registered
    MetadataMap list = r->list();
    EXPECT_EQ(2u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_EQ(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));

    // Copy testscopeC into the scopes folder
    std::cout << "Create testscopeC dir in the scopes folder" << std::endl;
    filesystem::create_directory(TEST_RUNTIME_PATH "/scopes/testscopeC", ec);
    ASSERT_EQ("Success", ec.message());

    reset();
    std::cout << "Copy testscopeC.ini into the scopes folder" << std::endl;
    filesystem::copy(TEST_RUNTIME_PATH "/other_scopes/testscopeC/testscopeC.ini", TEST_RUNTIME_PATH "/scopes/testscopeC/testscopeC.ini", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_update());

    // Now check that we have 3 scopes registered
    list = r->list();
    EXPECT_EQ(3u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));

    reset();
    // Make a symlink to testscopeD in the scopes folder
    std::cout << "Make a symlink to testscopeD in the scopes folder" << std::endl;
    filesystem::create_symlink(TEST_RUNTIME_PATH "/other_scopes/testscopeD", TEST_RUNTIME_PATH "/scopes/testscopeD", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_update());

    // The previous test may emit a single update or two, depending
    // on whether the copy operation manages to write data
    // into the .ini file quickly enough. (If so, we get
    // a single notification. If not, and inotify gives
    // us two events (one for creation and one for write/close),
    // we get two updates.) The race is unavoidable because
    // we *must* emit an update on file creation, because
    // a file can be linked (instead of copied) into the scope's
    // config dir. This means that calling reset() before calling
    // list() below does *not* fix the problem: the second update
    // from the previous test may arrive *after* we call reset().
    // So, we wait for up to five seconds for the list() operation
    // to return the correct result before failing.
    auto const wait_time = std::chrono::seconds(5);
    auto start_time = std::chrono::system_clock::now();
    bool ok = false;
    do
    {
        try
        {
            list = r->list();
            ok = list.size() == 4;
        }
        catch (std::exception const& e)
        {
            FAIL() << e.what();
        }
    }
    while (!ok && std::chrono::system_clock::now() - start_time < wait_time);
    EXPECT_TRUE(ok) << "new scope not recognized after " << wait_time.count() << " seconds";

    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeC"));
    EXPECT_NE(list.end(), list.find("testscopeD"));

    reset();
    // Remove testscopeC from the scopes folder
    std::cout << "Remove testscopeC from the scopes folder" << std::endl;
    filesystem::remove_all(TEST_RUNTIME_PATH "/scopes/testscopeC", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_update());

    // Now check that we have 3 scopes registered again
    list = r->list();
    EXPECT_EQ(3u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_EQ(list.end(), list.find("testscopeC"));
    EXPECT_NE(list.end(), list.find("testscopeD"));

    reset();
    // Remove symlink to testscopeD from the scopes folder
    std::cout << "Remove symlink to testscopeD from the scopes folder" << std::endl;
    filesystem::remove(TEST_RUNTIME_PATH "/scopes/testscopeD", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_update());

    // Now check that we are back to having 2 scopes registered
    list = r->list();
    EXPECT_EQ(2u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_EQ(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));

    // Make a folder in scopes named "testfolder"
    std::cout << "Make a folder in scopes named \"testfolder\"" << std::endl;
    filesystem::create_directory(TEST_RUNTIME_PATH "/scopes/testfolder", ec);
    ASSERT_EQ("Success", ec.message());

    // Check that no scopes were registered
    list = r->list();
    EXPECT_EQ(2u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_EQ(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));

    reset();
    // Make a symlink to testscopeC.ini in testfolder
    std::cout << "Make a symlink to testscopeC.ini in testfolder" << std::endl;
    filesystem::create_symlink(TEST_RUNTIME_PATH "/other_scopes/testscopeC/testscopeC.ini", TEST_RUNTIME_PATH "/scopes/testfolder/testscopeC.ini", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_update());

    // Now check that we have 3 scopes registered
    list = r->list();
    EXPECT_EQ(3u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_NE(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));

    reset();
    // Remove testfolder
    std::cout << "Remove testfolder" << std::endl;
    filesystem::remove_all(TEST_RUNTIME_PATH "/scopes/testfolder", ec);
    ASSERT_EQ("Success", ec.message());
    EXPECT_TRUE(wait_for_update());

    // Now check that we are back to having 2 scopes registered
    list = r->list();
    EXPECT_EQ(2u, list.size());
    EXPECT_NE(list.end(), list.find("testscopeA"));
    EXPECT_NE(list.end(), list.find("testscopeB"));
    EXPECT_EQ(list.end(), list.find("testscopeC"));
    EXPECT_EQ(list.end(), list.find("testscopeD"));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int rc = 0;

    // Unlink in case we left the link behind from an earlier interrupted run.
    system::error_code ec;
    filesystem::remove_all(TEST_RUNTIME_PATH "/scopes/testscopeC", ec);
    filesystem::remove_all(TEST_RUNTIME_PATH "/click",ec);
    filesystem::remove(TEST_RUNTIME_PATH "/scopes/testscopeD", ec);
    filesystem::remove_all(TEST_RUNTIME_PATH "/scopes/testfolder", ec);
    filesystem::remove(TEST_RUNTIME_PATH "/scopes/testscopeB/testscopeB-settings.ini", ec);

    // Set the "TEST_DESKTOP_FILES_DIR" env var before forking as not to create desktop files in ~/.local
    putenv(const_cast<char*>("TEST_DESKTOP_FILES_DIR=" TEST_RUNTIME_PATH));

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
        std::ifstream la("/proc/loadavg");
        std::string avg[3];
        la >> avg[0] >> avg[1] >> avg[2];
        std::cerr << "load average: " << avg[0] << " " << avg[1] << " " << avg[2] << std::endl;

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
