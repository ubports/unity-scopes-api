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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/OnlineAccountClient.h>
#include <unity/scopes/internal/OnlineAccountClientImpl.h>

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Variant.h>

#include <boost/filesystem/operations.hpp>
#include <condition_variable>
#include <functional>
#include <gtest/gtest.h>
#include <libaccounts-glib/accounts-glib.h>
#include <thread>

using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::testing;
using namespace unity::scopes::internal;

namespace unity
{
namespace scopes
{
namespace testing
{

class OnlineAccountClientTest : public ::testing::Test
{
public:
    OnlineAccountClientTest(OnlineAccountClient::MainLoopSelect main_loop_select = OnlineAccountClient::CreateInternalMainLoop)
    {
        boost::filesystem::remove(TEST_DB_DIR "/accounts.db");

        setenv("ACCOUNTS", TEST_DB_DIR, false);
        setenv("AG_SERVICES", TEST_DATA_DIR, false);
        setenv("AG_SERVICE_TYPES", TEST_DATA_DIR, false);
        setenv("AG_PROVIDERS", TEST_DATA_DIR, false);

        oa_client_.reset(new OnlineAccountClient("TestService", "sharing", "TestProvider", main_loop_select));

        manager_ = oa_client_->p->manager();
    }

    ~OnlineAccountClientTest()
    {
        account_ = nullptr;
        manager_ = nullptr;
        oa_client_ = nullptr;
    }

    std::shared_ptr<OnlineAccountClient> oa_client()
    {
        return oa_client_;
    }

    void create_account()
    {
        run_in_main_loop_(std::bind(&OnlineAccountClientTest::create_account_, this));
    }

    void delete_account()
    {
        run_in_main_loop_(std::bind(&OnlineAccountClientTest::delete_account_, this));
    }

    void enable_service()
    {
        run_in_main_loop_(std::bind(&OnlineAccountClientTest::enable_service_, this));
    }

    void disable_service()
    {
        run_in_main_loop_(std::bind(&OnlineAccountClientTest::disable_service_, this));
    }

    void disable_account()
    {
        run_in_main_loop_(std::bind(&OnlineAccountClientTest::disable_account_, this));
    }

    void enable_account()
    {
        run_in_main_loop_(std::bind(&OnlineAccountClientTest::enable_account_, this));
    }

    void service_update_enabled(OnlineAccountClient::ServiceStatus const& status)
    {
        EXPECT_EQ(1, status.account_id);
        EXPECT_EQ(true, status.service_enabled);
        EXPECT_EQ(false, status.service_authenticated);
        EXPECT_EQ("69842936499-sdflkbhslufhgrjamwlicefhb.apps.test.com", status.client_id);
        EXPECT_EQ("lj3i8iorep0w03994jwjef0j", status.client_secret);
        EXPECT_EQ("", status.access_token);
        EXPECT_EQ("", status.token_secret);
        EXPECT_NE("", status.error);

        std::lock_guard<std::mutex> lock(mutex_);
        got_update_ = true;
        cond_.notify_all();
    }

    void service_update_disabled(OnlineAccountClient::ServiceStatus const& status)
    {
        EXPECT_EQ(1, status.account_id);
        EXPECT_EQ(false, status.service_enabled);
        EXPECT_EQ(false, status.service_authenticated);
        EXPECT_EQ("69842936499-sdflkbhslufhgrjamwlicefhb.apps.test.com", status.client_id);
        EXPECT_EQ("lj3i8iorep0w03994jwjef0j", status.client_secret);
        EXPECT_EQ("", status.access_token);
        EXPECT_EQ("", status.token_secret);
        EXPECT_EQ("", status.error);

        std::lock_guard<std::mutex> lock(mutex_);
        got_update_ = true;
        cond_.notify_all();
    }

    void service_update_none(OnlineAccountClient::ServiceStatus const& status)
    {
        EXPECT_EQ(1, status.account_id);
        EXPECT_EQ(false, status.service_enabled);
        EXPECT_EQ(false, status.service_authenticated);
        EXPECT_EQ("", status.client_id);
        EXPECT_EQ("", status.client_secret);
        EXPECT_EQ("", status.access_token);
        EXPECT_EQ("", status.token_secret);
        EXPECT_EQ("", status.error);

        std::lock_guard<std::mutex> lock(mutex_);
        got_update_ = true;
        cond_.notify_all();
    }

    bool wait_for_service_update()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        EXPECT_TRUE(cond_.wait_for(lock, std::chrono::seconds(1), [this]{ return got_update_; })) << "service update callback not triggered";
        bool result = got_update_;
        got_update_ = false;
        return result;
    }

private:
    std::shared_ptr<OnlineAccountClient> oa_client_;
    std::shared_ptr<AgManager> manager_;
    std::shared_ptr<AgAccount> account_;

    bool got_update_;
    std::mutex mutex_;
    std::condition_variable cond_;

private:
    void run_in_main_loop_(std::function<void()> func)
    {
        std::shared_ptr<GMainLoop> event_loop;
        event_loop.reset(g_main_loop_new(nullptr, false), g_main_loop_unref);

        auto cb_data = std::make_pair(&func, event_loop.get());
        g_idle_add([](void* user_data)
        {
            auto cb_data = reinterpret_cast<std::pair<std::function<void()>*, GMainLoop*>*>(user_data);
            auto func = cb_data->first;
            auto event_loop = cb_data->second;

            (*func)();

            while (!g_main_loop_is_running(event_loop));
            g_main_loop_quit(event_loop);

            return G_SOURCE_REMOVE;
        }, &cb_data);

        g_main_loop_run(event_loop.get());
    }

    void create_account_()
    {
        account_.reset(ag_manager_create_account(manager_.get(), "TestProvider"), g_object_unref);
        ag_account_select_service (account_.get(), nullptr);
        ag_account_set_enabled(account_.get(), true);

        GError* error = nullptr;
        bool stored = ag_account_store_blocking(account_.get(), &error);
        std::shared_ptr<GError> error_cleanup(error, safe_g_error_free_);

        EXPECT_TRUE(stored);
        EXPECT_EQ(nullptr, error);
    }

    void delete_account_()
    {
        ag_account_delete(account_.get());

        GError* error = nullptr;
        bool stored = ag_account_store_blocking(account_.get(), &error);
        std::shared_ptr<GError> error_cleanup(error, safe_g_error_free_);

        account_ = nullptr;

        EXPECT_TRUE(stored);
        EXPECT_EQ(nullptr, error);
    }

    void enable_service_()
    {
        auto service = ag_manager_get_service (manager_.get(), "TestService");
        ag_account_select_service (account_.get(), service);
        ag_account_set_enabled (account_.get(), true);

        GError* error = nullptr;
        bool stored = ag_account_store_blocking(account_.get(), &error);
        std::shared_ptr<GError> error_cleanup(error, safe_g_error_free_);

        EXPECT_TRUE(stored);
        EXPECT_EQ(nullptr, error);
    }

    void disable_service_()
    {
        auto service = ag_manager_get_service (manager_.get(), "TestService");
        ag_account_select_service (account_.get(), service);
        ag_account_set_enabled (account_.get(), false);

        GError* error = nullptr;
        bool stored = ag_account_store_blocking(account_.get(), &error);
        std::shared_ptr<GError> error_cleanup(error, safe_g_error_free_);

        EXPECT_TRUE(stored);
        EXPECT_EQ(nullptr, error);
    }

    void disable_account_()
    {
        ag_account_set_enabled(account_.get(), false);

        GError* error = nullptr;
        bool stored = ag_account_store_blocking(account_.get(), &error);
        std::shared_ptr<GError> error_cleanup(error, safe_g_error_free_);

        EXPECT_TRUE(stored);
        EXPECT_EQ(nullptr, error);
    }

    void enable_account_()
    {
        ag_account_set_enabled(account_.get(), true);

        GError* error = nullptr;
        bool stored = ag_account_store_blocking(account_.get(), &error);
        std::shared_ptr<GError> error_cleanup(error, safe_g_error_free_);

        EXPECT_TRUE(stored);
        EXPECT_EQ(nullptr, error);
    }

    static void safe_g_error_free_(GError* e)
    {
        if (e)
        {
            g_error_free(e);
        }
    }
};

class OnlineAccountClientTestNoMainLoop : public OnlineAccountClientTest
{
public:
    OnlineAccountClientTestNoMainLoop()
        : OnlineAccountClientTest(OnlineAccountClient::RunInExternalMainLoop) {}
};

} // namespace testing
} // namespace scopes
} // namespace unity

TEST(OnlineAccountClient, register_account_login_result)
{
    OnlineAccountClient oa_client("test_service_name", "test_service_type", "test_provider");

    CategoryRegistry reg;
    auto cat = reg.register_category("1", "title", "icon", nullptr, CategoryRenderer());
    CategorisedResult result(cat);

    oa_client.register_account_login_item(result, CannedQuery("test"), OnlineAccountClient::InvalidateResults, OnlineAccountClient::DoNothing);

    EXPECT_TRUE(result.contains("online_account_details"));

    VariantMap details = result.value("online_account_details").get_dict();

    EXPECT_NE(details.end(), details.find("service_name"));
    EXPECT_NE(details.end(), details.find("service_type"));
    EXPECT_NE(details.end(), details.find("provider_name"));
    EXPECT_NE(details.end(), details.find("login_passed_action"));
    EXPECT_NE(details.end(), details.find("login_failed_action"));

    EXPECT_EQ("test_service_name", details.at("service_name").get_string());
    EXPECT_EQ("test_service_type", details.at("service_type").get_string());
    EXPECT_EQ("test_provider", details.at("provider_name").get_string());
    EXPECT_EQ(OnlineAccountClient::InvalidateResults, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_passed_action").get_int()));
    EXPECT_EQ(OnlineAccountClient::DoNothing, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_failed_action").get_int()));
}

TEST(OnlineAccountClient, register_account_login_widget)
{
    OnlineAccountClient oa_client("test_service_name", "test_service_type", "test_provider");

    PreviewWidget widget("i1", "image");

    oa_client.register_account_login_item(widget, OnlineAccountClient::ContinueActivation, OnlineAccountClient::DoNothing);

    VariantMap values = widget.attribute_values();

    EXPECT_NE(values.end(), values.find("online_account_details"));

    VariantMap details = values.at("online_account_details").get_dict();

    EXPECT_NE(details.end(), details.find("service_name"));
    EXPECT_NE(details.end(), details.find("service_type"));
    EXPECT_NE(details.end(), details.find("provider_name"));
    EXPECT_NE(details.end(), details.find("login_passed_action"));
    EXPECT_NE(details.end(), details.find("login_failed_action"));

    EXPECT_EQ("test_service_name", details.at("service_name").get_string());
    EXPECT_EQ("test_service_type", details.at("service_type").get_string());
    EXPECT_EQ("test_provider", details.at("provider_name").get_string());
    EXPECT_EQ(OnlineAccountClient::ContinueActivation, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_passed_action").get_int()));
    EXPECT_EQ(OnlineAccountClient::DoNothing, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_failed_action").get_int()));
}

///! Callback/manual create/delete, enable/disable account/service, signin

TEST_F(OnlineAccountClientTest, refresh_services_main_loop)
{
    auto statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(0, statuses.size());

    create_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(1, statuses.size());
    EXPECT_FALSE(statuses[0].service_enabled);

    enable_service();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_TRUE(statuses[0].service_enabled);

    disable_service();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_FALSE(statuses[0].service_enabled);

    enable_service();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_TRUE(statuses[0].service_enabled);

    disable_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_FALSE(statuses[0].service_enabled);

    enable_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_TRUE(statuses[0].service_enabled);

    delete_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(0, statuses.size());
}

TEST_F(OnlineAccountClientTestNoMainLoop, refresh_services_no_main_loop)
{
    auto statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(0, statuses.size());

    create_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(1, statuses.size());
    EXPECT_FALSE(statuses[0].service_enabled);

    enable_service();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_TRUE(statuses[0].service_enabled);

    disable_service();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_FALSE(statuses[0].service_enabled);

    enable_service();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_TRUE(statuses[0].service_enabled);

    disable_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_FALSE(statuses[0].service_enabled);

    enable_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_TRUE(statuses[0].service_enabled);
}

TEST_F(OnlineAccountClientTest, service_update_callback)
{
    auto statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(0, statuses.size());

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update_none, this, std::placeholders::_1));
    create_account();
    wait_for_service_update();

    statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(1, statuses.size());

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update_enabled, this, std::placeholders::_1));
    enable_service();
    wait_for_service_update();

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update_disabled, this, std::placeholders::_1));
    disable_service();
    wait_for_service_update();

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update_enabled, this, std::placeholders::_1));
    enable_service();
    wait_for_service_update();

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update_disabled, this, std::placeholders::_1));
    disable_account();
    wait_for_service_update();

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update_enabled, this, std::placeholders::_1));
    enable_account();
    wait_for_service_update();

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update_disabled, this, std::placeholders::_1));
    delete_account();
    wait_for_service_update();

    statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(0, statuses.size());
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    for (int i = 0; i < 50; ++i)
    {
        if (RUN_ALL_TESTS() != 0)
        {
            return -1;
        }
    }
    return 0;
}
