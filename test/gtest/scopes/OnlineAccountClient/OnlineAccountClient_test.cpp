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
#include <libaccounts-glib/accounts-glib.h>
#include <libdbustest/dbus-test.h>
#include <thread>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

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

        setenv("XDG_RUNTIME_DIR", "/tmp", true);
        setenv("ACCOUNTS", TEST_DB_DIR, false);
        setenv("AG_SERVICES", TEST_DATA_DIR, false);
        setenv("AG_SERVICE_TYPES", TEST_DATA_DIR, false);
        setenv("AG_PROVIDERS", TEST_DATA_DIR, false);

        VariantMap params;
        params["ClientId"] = "abcXYZ";
        params["Timeout"] = 200;
        params["UseSSL"] = true;
        params["Humidity"] = 0.3;
        params["Unsupported"] = VariantMap();
        oa_client_.reset(new OnlineAccountClient("TestService", "sharing", "TestProvider", params, main_loop_select));

        manager_ = oa_client_->p->manager();
        main_loop_context_ = oa_client_->p->main_loop_context();
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

    void service_update(OnlineAccountClient::ServiceStatus const& status)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        last_status_ = status;
        got_update_ = true;
        cond_.notify_all();

        // Call get_service_statuses() from within the callback to make sure it does not cause any deadlocks
        oa_client_->get_service_statuses();
    }

    bool wait_for_service_update(OnlineAccountClient::ServiceStatus const& status)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait_for(lock, std::chrono::seconds(1), [this]{ return got_update_; });
        if (got_update_)
        {
            EXPECT_EQ(status.account_id, last_status_.account_id);
            EXPECT_EQ(status.service_enabled, last_status_.service_enabled);
            EXPECT_EQ(status.service_authenticated, last_status_.service_authenticated);
            EXPECT_EQ(status.client_id, last_status_.client_id);
            EXPECT_EQ(status.client_secret, last_status_.client_secret);
            EXPECT_EQ(status.access_token, last_status_.access_token);
            EXPECT_EQ(status.token_secret, last_status_.token_secret);
            EXPECT_EQ(status.error, last_status_.error);

            got_update_ = false;
            return true;
        }
        else
        {
            got_update_ = false;
            return false;
        }
    }

    void invoke_callback(std::shared_ptr<OnlineAccountClient> oa_client, AccountInfo* info, std::string const& error)
    {
        oa_client->p->callback(info, error);
    }

    static void safe_g_error_free_(GError* e)
    {
        if (e)
        {
            g_error_free(e);
        }
    }

    static void safe_g_variant_free_(GVariant* v)
    {
        if (v)
        {
            g_variant_unref(v);
        }
    }

private:
    std::shared_ptr<OnlineAccountClient> oa_client_;
    std::shared_ptr<AgManager> manager_;
    std::shared_ptr<AgAccount> account_;
    std::shared_ptr<GMainContext> main_loop_context_;

    OnlineAccountClient::ServiceStatus last_status_;
    bool got_update_ = false;
    std::mutex mutex_;
    std::condition_variable cond_;

private:
    void run_in_main_loop_(std::function<void()> func)
    {
        auto func_task = [](void* user_data)
        {
            auto cb_data = reinterpret_cast<std::pair<std::function<void()>*, GMainLoop*>*>(user_data);
            auto func = cb_data->first;
            auto event_loop = cb_data->second;

            (*func)();

            while (!g_main_loop_is_running(event_loop));
            g_main_loop_quit(event_loop);

            return G_SOURCE_REMOVE;
        };

        std::shared_ptr<GMainLoop> event_loop;
        if (main_loop_context_)
        {
            event_loop.reset(g_main_loop_new(main_loop_context_.get(), false), g_main_loop_unref);
            auto cb_data = std::make_pair(&func, event_loop.get());
            g_main_context_invoke(main_loop_context_.get(), func_task, &cb_data);
        }
        else
        {
            event_loop.reset(g_main_loop_new(nullptr, false), g_main_loop_unref);
            auto cb_data = std::make_pair(&func, event_loop.get());
            g_idle_add(func_task, &cb_data);
        }
        g_main_loop_run(event_loop.get());
    }

    void create_account_()
    {
        account_.reset(ag_manager_create_account(manager_.get(), "TestProvider"), g_object_unref);
        ag_account_select_service(account_.get(), nullptr);
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
        auto service = ag_manager_get_service(manager_.get(), "TestService");
        ag_account_select_service(account_.get(), service);
        ag_account_set_enabled(account_.get(), true);

        GError* error = nullptr;
        bool stored = ag_account_store_blocking(account_.get(), &error);
        std::shared_ptr<GError> error_cleanup(error, safe_g_error_free_);

        EXPECT_TRUE(stored);
        EXPECT_EQ(nullptr, error);
    }

    void disable_service_()
    {
        auto service = ag_manager_get_service(manager_.get(), "TestService");
        ag_account_select_service(account_.get(), service);
        ag_account_set_enabled(account_.get(), false);

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
};

class OnlineAccountClientTestNoMainLoop : public OnlineAccountClientTest
{
public:
    OnlineAccountClientTestNoMainLoop()
        : OnlineAccountClientTest(OnlineAccountClient::RunInExternalMainLoop) {}
};

class SignondMock
{
public:
    SignondMock()
        : mock_(dbus_test_dbus_mock_new("com.google.code.AccountsSSO.SingleSignOn"))
    {
        DbusTestDbusMockObject* authService =
            dbus_test_dbus_mock_get_object(mock_,
                                           "/com/google/code/AccountsSSO/SingleSignOn",
                                           "com.google.code.AccountsSSO.SingleSignOn.AuthService",
                                           NULL);

        dbus_test_dbus_mock_object_add_method(mock_, authService,
                                              "getAuthSessionObjectPath",
                                              G_VARIANT_TYPE("(us)"),
                                              G_VARIANT_TYPE("s"),
                                              "ret = '/AuthSession'",
                                              NULL);

        DbusTestDbusMockObject* authSession =
            dbus_test_dbus_mock_get_object(mock_,
                                           "/AuthSession",
                                           "com.google.code.AccountsSSO.SingleSignOn.AuthSession",
                                           NULL);

        static const char* const process_code =
            "data = dict(args[0])\n"
            "if 'ClientId' in data:\n"
            "    data['AccessToken'] = data['ClientId'].swapcase()\n"
            "ret = data\n";
        dbus_test_dbus_mock_object_add_method(mock_, authSession,
                                              "process",
                                              G_VARIANT_TYPE("(a{sv}s)"),
                                              G_VARIANT_TYPE("a{sv}"),
                                              process_code,
                                              NULL);
    }

    DbusTestTask* task() const { return DBUS_TEST_TASK(mock_); }

private:
    DbusTestDbusMock* mock_;
};

} // namespace testing
} // namespace scopes
} // namespace unity

TEST_F(OnlineAccountClientTest, register_account_login_result)
{
    VariantMap params;
    params["ClientId"] = "abc";
    params["Timeout"] = 200;
    params["UseSSL"] = true;
    OnlineAccountClient oa_client("test_service_name", "test_service_type", "test_provider", params);

    CategoryRegistry reg;
    auto cat = reg.register_category("1", "title", "icon", nullptr, CategoryRenderer());
    CategorisedResult result(cat);

    EXPECT_FALSE(result.is_account_login_result());
    oa_client.register_account_login_item(result, CannedQuery("test"), OnlineAccountClient::InvalidateResults, OnlineAccountClient::DoNothing);
    EXPECT_TRUE(result.is_account_login_result());

    EXPECT_TRUE(result.contains("online_account_details"));

    VariantMap details = result.value("online_account_details").get_dict();

    EXPECT_NE(details.end(), details.find("scope_id"));
    EXPECT_NE(details.end(), details.find("service_name"));
    EXPECT_NE(details.end(), details.find("service_type"));
    EXPECT_NE(details.end(), details.find("provider_name"));
    EXPECT_NE(details.end(), details.find("auth_params"));
    EXPECT_NE(details.end(), details.find("login_passed_action"));
    EXPECT_NE(details.end(), details.find("login_failed_action"));

    EXPECT_EQ("test", details.at("scope_id").get_string());
    EXPECT_EQ("test_service_name", details.at("service_name").get_string());
    EXPECT_EQ("test_service_type", details.at("service_type").get_string());
    EXPECT_EQ("test_provider", details.at("provider_name").get_string());
    VariantMap actual_params = details.at("auth_params").get_dict();
    EXPECT_EQ("abc", actual_params.at("ClientId").get_string());
    EXPECT_EQ(200, actual_params.at("Timeout").get_int());
    EXPECT_EQ(true, actual_params.at("UseSSL").get_bool());
    EXPECT_EQ(OnlineAccountClient::InvalidateResults, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_passed_action").get_int()));
    EXPECT_EQ(OnlineAccountClient::DoNothing, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_failed_action").get_int()));
}

TEST_F(OnlineAccountClientTest, register_account_login_widget)
{
    OnlineAccountClient oa_client("test_service_name", "test_service_type", "test_provider");

    PreviewWidget widget("i1", "image");

    oa_client.register_account_login_item(widget, OnlineAccountClient::ContinueActivation, OnlineAccountClient::DoNothing);

    VariantMap values = widget.attribute_values();

    EXPECT_NE(values.end(), values.find("online_account_details"));

    VariantMap details = values.at("online_account_details").get_dict();

    EXPECT_NE(details.end(), details.find("scope_id"));
    EXPECT_NE(details.end(), details.find("service_name"));
    EXPECT_NE(details.end(), details.find("service_type"));
    EXPECT_NE(details.end(), details.find("provider_name"));
    EXPECT_NE(details.end(), details.find("login_passed_action"));
    EXPECT_NE(details.end(), details.find("login_failed_action"));

    EXPECT_EQ("", details.at("scope_id").get_string());
    EXPECT_EQ("test_service_name", details.at("service_name").get_string());
    EXPECT_EQ("test_service_type", details.at("service_type").get_string());
    EXPECT_EQ("test_provider", details.at("provider_name").get_string());
    EXPECT_EQ(OnlineAccountClient::ContinueActivation, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_passed_action").get_int()));
    EXPECT_EQ(OnlineAccountClient::DoNothing, static_cast<OnlineAccountClient::PostLoginAction>(details.at("login_failed_action").get_int()));
}

TEST_F(OnlineAccountClientTest, refresh_services_main_loop)
{
    auto statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(0u, statuses.size());

    create_account();
    oa_client()->refresh_service_statuses();

    statuses = oa_client()->get_service_statuses();
    EXPECT_EQ(1u, statuses.size());
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
    EXPECT_EQ(0u, statuses.size());
}

TEST_F(OnlineAccountClientTest, service_update_callback)
{
    OnlineAccountClient::ServiceStatus none_status;
    none_status.account_id = 1;
    none_status.service_enabled = false;
    none_status.service_authenticated = false;
    none_status.client_id = "";
    none_status.client_secret = "";
    none_status.access_token = "";
    none_status.token_secret = "";
    none_status.error = "";

    OnlineAccountClient::ServiceStatus enabled_status;
    enabled_status.account_id = 1;
    enabled_status.service_enabled = true;
    enabled_status.service_authenticated = true;
    enabled_status.client_id = "abcXYZ";
    enabled_status.client_secret = "lj3i8iorep0w03994jwjef0j";
    enabled_status.access_token = "ABCxyz";
    enabled_status.token_secret = "";
    enabled_status.error = "";

    OnlineAccountClient::ServiceStatus disabled_status;
    disabled_status.account_id = 1;
    disabled_status.service_enabled = false;
    disabled_status.service_authenticated = false;
    disabled_status.client_id = "abcXYZ";
    disabled_status.client_secret = "lj3i8iorep0w03994jwjef0j";
    disabled_status.access_token = "";
    disabled_status.token_secret = "";
    disabled_status.error = "";

    create_account();
    EXPECT_FALSE(wait_for_service_update(none_status));

    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update, this, std::placeholders::_1));
    EXPECT_TRUE(wait_for_service_update(none_status));

    enable_service();
    EXPECT_TRUE(wait_for_service_update(enabled_status));

    disable_service();
    EXPECT_TRUE(wait_for_service_update(disabled_status));

    enable_service();
    EXPECT_TRUE(wait_for_service_update(enabled_status));

    disable_account();
    EXPECT_TRUE(wait_for_service_update(disabled_status));

    enable_account();
    EXPECT_TRUE(wait_for_service_update(enabled_status));

    delete_account();
    EXPECT_TRUE(wait_for_service_update(disabled_status));
}

TEST_F(OnlineAccountClientTestNoMainLoop, authentication)
{
    OnlineAccountClient::ServiceStatus auth_status;
    auth_status.account_id = 1;
    auth_status.service_enabled = true;
    auth_status.service_authenticated = true;
    auth_status.client_id = "isuertbiseruy87srkuthksvu";
    auth_status.client_secret = "rytwekfgiodng523dr4";
    auth_status.access_token = "sfhgbfgutgi9ugwirheg74";
    auth_status.token_secret = "qwpeurylsfdg83";
    auth_status.error = "not really an error, but just to test";

    std::shared_ptr<AccountInfo> info(new AccountInfo);
    info->account_id = 1;
    info->service_enabled = true;
    {
        GVariantDict dict;
        g_variant_dict_init(&dict, nullptr);
        g_variant_dict_insert(&dict, "ConsumerKey", "s", "isuertbiseruy87srkuthksvu");
        g_variant_dict_insert(&dict, "ConsumerSecret", "s", "rytwekfgiodng523dr4");
        info->auth_params.reset(g_variant_ref_sink(g_variant_dict_end(&dict)), safe_g_variant_free_);
    }
    {
        GVariantDict dict;
        g_variant_dict_init(&dict, nullptr);
        g_variant_dict_insert(&dict, "AccessToken", "s", "sfhgbfgutgi9ugwirheg74");
        g_variant_dict_insert(&dict, "TokenSecret", "s", "qwpeurylsfdg83");
        info->session_data.reset(g_variant_ref_sink(g_variant_dict_end(&dict)), safe_g_variant_free_);
    }

    // Manually invoke the callback with a valid access token, which should result in service_authenticated = true
    oa_client()->set_service_update_callback(std::bind(&OnlineAccountClientTest::service_update, this, std::placeholders::_1));
    invoke_callback(oa_client(), info.get(), "not really an error, but just to test");
    EXPECT_TRUE(wait_for_service_update(auth_status));
}

int main(int argc, char **argv)
{
    std::shared_ptr<DbusTestService> dbus_test_service;
    dbus_test_service.reset(dbus_test_service_new(nullptr), g_object_unref);

    unity::scopes::testing::SignondMock signondMock;
    dbus_test_service_add_task(dbus_test_service.get(), signondMock.task());

    dbus_test_service_start_tasks(dbus_test_service.get());

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
