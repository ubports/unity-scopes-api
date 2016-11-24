/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
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

#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/scopes/internal/Logger.h>
#include <unity/scopes/internal/smartscopes/HttpClientNetCpp.h>
#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>
#include <unity/scopes/OptionSelectorFilter.h>

#include <unity/UnityExceptions.h>
#include <unity/util/FileIO.h>
#include <boost/filesystem/operations.hpp>
#include <fstream>

#include "../RaiiServer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <memory>
#include <thread>

using namespace testing;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;
using namespace unity::test::scopes::internal::smartscopes;

namespace
{

class SmartScopesClientTest : public Test
{
public:
    SmartScopesClientTest()
        : http_client_(new HttpClientNetCpp(20000)),
          json_node_(new JsonCppNode()),
          server_(FAKE_SSS_PATH, FAKE_SSS_LOG)
    {
        boost::filesystem::remove(FAKE_SSS_LOG);
        sss_url_ = "http://127.0.0.1:" + std::to_string(server_.port_);
        ssc_ = std::make_shared<SmartScopesClient>(http_client_, json_node_, nullptr, sss_url_, PARTNER_FILE);
    }

    bool grep_string(std::string const &s)
    {
        std::stringstream str(unity::util::read_text_file(FAKE_SSS_LOG));
        while (str)
        {
            char tmp[1024];
            str.getline(tmp, 1024);
            if (strstr(tmp, s.c_str()))
            {
                return true;
            }
        }
        return false;
    }

protected:
    std::string sss_url_;
    HttpClientInterface::SPtr http_client_;
    JsonNodeInterface::SPtr json_node_;
    SmartScopesClient::SPtr ssc_;
    RaiiServer server_;
};

TEST_F(SmartScopesClientTest, remote_scopes)
{
    std::vector<RemoteScope> scopes;

    // first try an invalid locale (should throw)
    EXPECT_THROW(ssc_->get_remote_scopes(scopes, "test_FAIL", false), std::exception);
    ASSERT_EQ(0u, scopes.size());

    // now try an empty locale
    EXPECT_TRUE(ssc_->get_remote_scopes(scopes, "", false));
    ASSERT_EQ(4u, scopes.size());

    // now try a valid locale
    EXPECT_TRUE(ssc_->get_remote_scopes(scopes, "test_TEST", false));
    ASSERT_EQ(4u, scopes.size());

    EXPECT_EQ("dummy.scope", scopes[0].id);
    EXPECT_EQ("Dummy Demo Scope", scopes[0].name);
    EXPECT_EQ("Dummy demo scope.", scopes[0].description);
    EXPECT_EQ("Mr.Fake", scopes[0].author);
    EXPECT_EQ(sss_url_ + "/demo", scopes[0].base_url);
    EXPECT_EQ("icon", *scopes[0].icon);
    EXPECT_EQ(nullptr, scopes[0].art);
    EXPECT_FALSE(scopes[0].invisible);
    EXPECT_EQ(0, scopes[0].version);
    EXPECT_EQ(nullptr, scopes[0].appearance);
    EXPECT_EQ(nullptr, scopes[0].settings);
    EXPECT_TRUE(scopes[0].keywords.empty());

    EXPECT_EQ("dummy.scope.2", scopes[1].id);
    EXPECT_EQ("Dummy Demo Scope 2", scopes[1].name);
    EXPECT_EQ("Dummy demo scope 2.", scopes[1].description);
    EXPECT_EQ("Mr.Fake", scopes[1].author);
    EXPECT_EQ(sss_url_ + "/demo2", scopes[1].base_url);
    EXPECT_EQ(nullptr, scopes[1].icon);
    EXPECT_EQ("art", *scopes[1].art);
    EXPECT_TRUE(scopes[1].invisible);
    EXPECT_EQ(2, scopes[1].version);
    EXPECT_EQ("#00BEEF", (*scopes[1].appearance)["background"].get_string());
    EXPECT_EQ("logo.png", (*scopes[1].appearance)["PageHeader"].get_dict()["logo"].get_string());
    EXPECT_EQ(nullptr, scopes[1].settings);
    EXPECT_TRUE(scopes[1].keywords.empty());

    EXPECT_EQ("dummy.scope.3", scopes[2].id);
    EXPECT_EQ("Dummy Demo Scope 3", scopes[2].name);
    EXPECT_EQ("Dummy demo scope 3.", scopes[2].description);
    EXPECT_EQ("Mr.Fake", scopes[2].author);
    EXPECT_EQ(sss_url_ + "/demo3", scopes[2].base_url);
    EXPECT_EQ(nullptr, scopes[2].icon);
    EXPECT_EQ(nullptr, scopes[2].art);
    EXPECT_FALSE(scopes[2].invisible);
    EXPECT_EQ(nullptr, scopes[2].appearance);
    {
        JsonCppNode node(*scopes[2].settings);
        Variant v = node.to_variant();

        VariantMap vm = v.get_array()[0].get_dict();
        EXPECT_EQ("Location", vm["displayName"].get_string());
        EXPECT_EQ("location", vm["id"].get_string());
        EXPECT_EQ("string", vm["type"].get_string());
        VariantMap params = vm["parameters"].get_dict();
        EXPECT_EQ("London", params["defaultValue"].get_string());

        vm = v.get_array()[1].get_dict();
        EXPECT_EQ("Temperature Units", vm["displayName"].get_string());
        EXPECT_EQ("unitTemp", vm["id"].get_string());
        EXPECT_EQ("list", vm["type"].get_string());
        params = vm["parameters"].get_dict();
        EXPECT_EQ(1, params["defaultValue"].get_int());
        VariantArray choices = params["values"].get_array();
        EXPECT_EQ("Celsius", choices[0].get_string());
        EXPECT_EQ("Fahrenheit", choices[1].get_string());

        vm = v.get_array()[2].get_dict();
        EXPECT_EQ("Age", vm["displayName"].get_string());
        EXPECT_EQ("age", vm["id"].get_string());
        EXPECT_EQ("number", vm["type"].get_string());
        params = vm["parameters"].get_dict();
        EXPECT_EQ(23, params["defaultValue"].get_int());

        vm = v.get_array()[3].get_dict();
        EXPECT_EQ("Enabled", vm["displayName"].get_string());
        EXPECT_EQ("enabled", vm["id"].get_string());
        EXPECT_EQ("boolean", vm["type"].get_string());
        params = vm["parameters"].get_dict();
        EXPECT_TRUE(params["defaultValue"].get_bool());
    }
    ASSERT_EQ(4u, scopes[2].keywords.size());
    EXPECT_NE(scopes[2].keywords.end(), scopes[2].keywords.find("music"));
    EXPECT_NE(scopes[2].keywords.end(), scopes[2].keywords.find("video"));
    EXPECT_NE(scopes[2].keywords.end(), scopes[2].keywords.find("news"));
    EXPECT_NE(scopes[2].keywords.end(), scopes[2].keywords.find("games"));

    EXPECT_TRUE(grep_string("/remote-scopes : partner=Partner%20String"));
}

TEST_F(SmartScopesClientTest, remote_scopes_no_partner)
{
    std::vector<RemoteScope> scopes;
    auto ssc_no_partner_ = std::make_shared<SmartScopesClient>(http_client_, json_node_, nullptr, sss_url_, "/this/file/doesnt/exist");
    EXPECT_TRUE(ssc_no_partner_->get_remote_scopes(scopes, "", false));
    EXPECT_FALSE(grep_string("/remote-scopes : partner"));
}

TEST_F(SmartScopesClientTest, search)
{
    std::vector<SearchResult> results;
    std::shared_ptr<DepartmentInfo> dept;
    Filters filters;
    FilterState filter_state;
    std::vector<std::shared_ptr<SearchCategory>> categories;

    SearchReplyHandler handler;
    handler.filters_handler = [&filters](Filters const &f) {
        filters = f;
    };
    handler.filter_state_handler = [&filter_state](FilterState const& s) {
        filter_state = s;
    };
    handler.category_handler = [&categories](std::shared_ptr<SearchCategory> const& cat) {
        categories.push_back(cat);
    };
    handler.result_handler = [&results](SearchResult const& result) {
        results.push_back(result);
    };
    handler.departments_handler = [&dept](std::shared_ptr<DepartmentInfo> const& deptinfo) {
        dept = deptinfo;
    };

    auto search_handle = ssc_->search(handler, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform", VariantMap(), VariantMap(), "en_US", LocationInfo(), "ThisIsUserAgentHeader");
    search_handle->wait();

    ASSERT_EQ(3u, results.size());
    ASSERT_EQ(1u, categories.size());

    EXPECT_EQ("URI", results[0].uri);
    EXPECT_EQ(nullptr, results[0].other_params["dnd_uri"]);
    EXPECT_EQ("Stuff", results[0].other_params["title"]->as_string());
    EXPECT_EQ(nullptr, results[0].other_params["icon"]);
    EXPECT_EQ("https://dash.ubuntu.com/imgs/amazon.png", results[0].other_params["art"]->as_string());
    EXPECT_EQ("cat1", results[0].category_id);

    EXPECT_EQ("cat1", categories[0]->id);
    EXPECT_EQ("Category 1", categories[0]->title);
    EXPECT_EQ("", categories[0]->icon);
    EXPECT_EQ("{}", categories[0]->renderer_template);

    EXPECT_EQ("URI2", results[1].uri);
    EXPECT_EQ(nullptr, results[1].other_params["dnd_uri"]);
    EXPECT_EQ("Things", results[1].other_params["title"]->as_string());
    EXPECT_EQ("https://dash.ubuntu.com/imgs/google.png", results[1].other_params["icon"]->as_string());
    EXPECT_EQ(nullptr, results[1].other_params["art"]);
    EXPECT_EQ("cat1", results[1].category_id);

    EXPECT_EQ("URI3", results[2].uri);
    EXPECT_EQ(nullptr, results[2].other_params["dnd_uri"]);
    EXPECT_EQ("Category Fail", results[2].other_params["title"]->as_string());
    EXPECT_EQ(nullptr, results[2].other_params["icon"]);
    EXPECT_EQ("https://dash.ubuntu.com/imgs/cat_fail.png", results[2].other_params["art"]->as_string());

    // check departments
    EXPECT_TRUE(dept != nullptr);
    EXPECT_EQ("All", dept->label);
    EXPECT_EQ("Foo", dept->alternate_label);
    EXPECT_FALSE(dept->has_subdepartments);

    auto const subdepts = dept->subdepartments;
    EXPECT_EQ(2u, subdepts.size());
    EXPECT_EQ("A", subdepts[0]->label);
    EXPECT_EQ("", subdepts[0]->alternate_label);
    EXPECT_EQ("scope://foo?q=&dep=a", subdepts[0]->canned_query);
    EXPECT_FALSE(subdepts[0]->has_subdepartments);

    {
        auto const subdepts_a = subdepts[0]->subdepartments;
        EXPECT_EQ(1u, subdepts_a.size());
        EXPECT_EQ("C", subdepts_a[0]->label);
        EXPECT_EQ("", subdepts_a[0]->alternate_label);
        EXPECT_EQ("scope://foo?q=&dep=c", subdepts_a[0]->canned_query);
    }

    EXPECT_EQ("B", subdepts[1]->label);
    EXPECT_EQ("scope://foo?q=&dep=b", subdepts[1]->canned_query);
    EXPECT_FALSE(subdepts[1]->has_subdepartments);

    // check filters
    EXPECT_FALSE(filters.empty());
    EXPECT_EQ(1u, filters.size());
    auto filter1 = filters.front();
    auto option_filter = std::dynamic_pointer_cast<const OptionSelectorFilter>(filter1);
    EXPECT_TRUE(option_filter != nullptr);
    EXPECT_EQ("Label", option_filter->label());
    EXPECT_EQ("sorting_primary_filter", option_filter->id());
    EXPECT_EQ(FilterBase::DisplayHints::Primary, option_filter->display_hints());
    EXPECT_FALSE(option_filter->multi_select());

    auto options = option_filter->options();
    EXPECT_EQ(3u, options.size());
    EXPECT_EQ("titlerank", options.front()->id());
    EXPECT_EQ("Title rank", options.front()->label());
    EXPECT_EQ("salesrank", options.back()->id());
    EXPECT_EQ("Bestselling", options.back()->label());

    EXPECT_TRUE(option_filter->has_active_option(filter_state));
    auto active_options = option_filter->active_options(filter_state);
    EXPECT_FALSE(active_options.empty());
    auto active_option = *(active_options.begin());
    EXPECT_EQ("salesrank", active_option->id());
}

TEST_F(SmartScopesClientTest, filter_groups)
{
    std::vector<SearchResult> results;
    std::shared_ptr<DepartmentInfo> dept;
    Filters filters;
    FilterState filter_state;
    std::vector<std::shared_ptr<SearchCategory>> categories;

    SearchReplyHandler handler;
    handler.filters_handler = [&filters](Filters const &f) {
        filters = f;
    };
    handler.filter_state_handler = [&filter_state](FilterState const& s) {
        filter_state = s;
    };
    handler.category_handler = [&categories](std::shared_ptr<SearchCategory> const& cat) {
        categories.push_back(cat);
    };
    handler.result_handler = [&results](SearchResult const& result) {
        results.push_back(result);
    };
    handler.departments_handler = [&dept](std::shared_ptr<DepartmentInfo> const& deptinfo) {
        dept = deptinfo;
    };

    auto search_handle = ssc_->search(handler, sss_url_ + "/demo", "filter_groups", "", "session_id", 0, "platform", VariantMap(), VariantMap(), "en_US", LocationInfo(), "ThisIsUserAgentHeader");
    search_handle->wait();

    ASSERT_EQ(1u, results.size());
    ASSERT_EQ(1u, categories.size());

    EXPECT_EQ("URI", results[0].uri);
    EXPECT_EQ("cat1", categories[0]->id);

    EXPECT_EQ(nullptr, nullptr);

    // check filters
    EXPECT_FALSE(filters.empty());
    EXPECT_EQ(1u, filters.size());
    auto filter1 = filters.front();
    auto option_filter = std::dynamic_pointer_cast<const OptionSelectorFilter>(filter1);
    EXPECT_TRUE(option_filter != nullptr);
    auto grp = option_filter->filter_group();
    EXPECT_TRUE(grp != nullptr);
    EXPECT_EQ("g1", grp->id());
    EXPECT_EQ("Group 1", grp->label());
    EXPECT_EQ("Label", option_filter->label());
    EXPECT_EQ("sorting_primary_filter", option_filter->id());
    EXPECT_EQ(FilterBase::DisplayHints::Primary, option_filter->display_hints());
    EXPECT_FALSE(option_filter->multi_select());

    auto options = option_filter->options();
    EXPECT_EQ(2u, options.size());

    EXPECT_TRUE(option_filter->has_active_option(filter_state));
    auto active_options = option_filter->active_options(filter_state);
    EXPECT_FALSE(active_options.empty());
    auto active_option = *(active_options.begin());
    EXPECT_EQ("salesrank", active_option->id());
}

TEST_F(SmartScopesClientTest, userAgentHeader)
{
    std::vector<SearchResult> results;

    SearchReplyHandler handler;
    handler.filters_handler = [](Filters const &) {
    };
    handler.filter_state_handler = [](FilterState const&) {
    };
    handler.category_handler = [](std::shared_ptr<SearchCategory> const&) {
    };
    handler.result_handler = [&results](SearchResult const& result) {
        results.push_back(result);
    };
    handler.departments_handler = [](std::shared_ptr<DepartmentInfo> const&) {
    };

    auto search_handle = ssc_->search(handler, sss_url_ + "/demo", "test_user_agent_header", "", "session_id", 0, "platform", VariantMap(), VariantMap(), "en_US", LocationInfo(), "ThisIsUserAgentHeader");
    search_handle->wait();

    ASSERT_EQ(4u, results.size());

    // user agent string is expected in the result title
    EXPECT_EQ("ThisIsUserAgentHeader", results[3].other_params["title"]->as_string());
}

TEST_F(SmartScopesClientTest, preview)
{
    PreviewReplyHandler handler;
    PreviewHandle::Columns columns;
    std::vector<std::string> widgets;
    handler.widget_handler = [&widgets](std::string const& widget_json) {
        widgets.push_back(widget_json);
    };
    handler.columns_handler = [&columns](PreviewHandle::Columns const &cols) {
        columns = cols;
    };

    auto preview_handle = ssc_->preview(handler, sss_url_ + "/demo", "result", "session_id", "platform", 0);
    preview_handle->wait();

    ASSERT_EQ(3u, columns.size());

    // column 1
    ASSERT_EQ(1u, columns[0].size());
    ASSERT_EQ(3u, columns[0][0].size());
    EXPECT_EQ("widget_id_A", columns[0][0][0]);
    EXPECT_EQ("widget_id_B", columns[0][0][1]);
    EXPECT_EQ("widget_id_C", columns[0][0][2]);

    // column 2
    ASSERT_EQ(2u, columns[1].size());
    ASSERT_EQ(1u, columns[1][0].size());
    EXPECT_EQ("widget_id_A", columns[1][0][0]);

    ASSERT_EQ(2u, columns[1][1].size());
    EXPECT_EQ("widget_id_B", columns[1][1][0]);
    EXPECT_EQ("widget_id_C", columns[1][1][1]);

    // column 3
    ASSERT_EQ(3u, columns[2].size());
    ASSERT_EQ(1u, columns[2][0].size());
    EXPECT_EQ("widget_id_A", columns[2][0][0]);

    ASSERT_EQ(1u, columns[2][1].size());
    EXPECT_EQ("widget_id_B", columns[2][1][0]);

    ASSERT_EQ(1u, columns[2][2].size());
    EXPECT_EQ("widget_id_C", columns[2][2][0]);

    ASSERT_EQ(3u, widgets.size());
    {
        auto json = widgets[0];
        JsonCppNode node(json);
        EXPECT_EQ("widget_id_A", node.get_node("id")->as_string());
        EXPECT_EQ("First widget.", node.get_node("text")->as_string());
        EXPECT_EQ("Widget A", node.get_node("title")->as_string());
        EXPECT_EQ("text", node.get_node("type")->as_string());
    }
    {
        auto json = widgets[1];
        JsonCppNode node(json);
        EXPECT_EQ("widget_id_B", node.get_node("id")->as_string());
        EXPECT_EQ("Second widget.", node.get_node("text")->as_string());
        EXPECT_EQ("Widget B", node.get_node("title")->as_string());
        EXPECT_EQ("text", node.get_node("type")->as_string());
    }
    {
        auto json = widgets[2];
        JsonCppNode node(json);
        EXPECT_EQ("widget_id_C", node.get_node("id")->as_string());
        EXPECT_EQ("Third widget.", node.get_node("text")->as_string());
        EXPECT_EQ("Widget C", node.get_node("title")->as_string());
        EXPECT_EQ("text", node.get_node("type")->as_string());
    }
}

TEST_F(SmartScopesClientTest, consecutive_searches)
{
    SearchReplyHandler handler1, handler2, handler3, handler4, handler5;
    std::vector<SearchResult> results1, results2, results3, results4, results5;

    handler1.filters_handler = [](Filters const &) {};
    handler1.filter_state_handler = [](FilterState const&) {};
    handler1.category_handler = [](std::shared_ptr<SearchCategory> const&) {};
    handler1.departments_handler = [](std::shared_ptr<DepartmentInfo> const&) {};

    handler2 = handler3 = handler4 = handler5 = handler1;

    handler1.result_handler = [&results1](SearchResult const& result) { results1.push_back(result); };
    handler2.result_handler = [&results2](SearchResult const& result) { results2.push_back(result); };
    handler3.result_handler = [&results3](SearchResult const& result) { results3.push_back(result); };
    handler4.result_handler = [&results4](SearchResult const& result) { results4.push_back(result); };
    handler5.result_handler = [&results5](SearchResult const& result) { results5.push_back(result); };

    auto search_handle1 = ssc_->search(handler1, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle2 = ssc_->search(handler2, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle3 = ssc_->search(handler3, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle4 = ssc_->search(handler4, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle5 = ssc_->search(handler5, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");

    search_handle1->wait();
    EXPECT_EQ(3u, results1.size());

    search_handle2->wait();
    EXPECT_EQ(3u, results2.size());

    search_handle3->wait();
    EXPECT_EQ(3u, results3.size());

    search_handle4->wait();
    EXPECT_EQ(3u, results4.size());

    search_handle5->wait();
    EXPECT_EQ(3u, results5.size());
}

TEST_F(SmartScopesClientTest, consecutive_cancels)
{
    SearchReplyHandler handler;
    handler.filters_handler = [](Filters const &) {};
    handler.filter_state_handler = [](FilterState const&) {};
    handler.category_handler = [](std::shared_ptr<SearchCategory> const&) {};
    handler.result_handler = [](SearchResult const&) {};
    handler.departments_handler = [](std::shared_ptr<DepartmentInfo> const&) {};

    for (int i = 0; i < 50; ++i)
    {
        auto search_handle = ssc_->search(handler, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
        search_handle->cancel_search();
        EXPECT_THROW(search_handle->wait(), std::exception);
    }

    SearchReplyHandler handler2;
    std::vector<SearchResult> results;
    handler2.filters_handler = [](Filters const &) {};
    handler2.filter_state_handler = [](FilterState const&) {};
    handler2.category_handler = [](std::shared_ptr<SearchCategory> const&) {};
    handler2.result_handler = [&results](SearchResult const& result) { results.push_back(result); };
    handler2.departments_handler = [](std::shared_ptr<DepartmentInfo> const&) {};

    auto search_handle = ssc_->search(handler2, sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");

    search_handle->wait();
    EXPECT_EQ(3u, results.size());
}

TEST_F(SmartScopesClientTest, reset_url)
{
    // check initial values to be expected test values
    EXPECT_EQ(sss_url_, ssc_->url());

    // empty the environment var (in case there already is one set)
    std::string server_url_env = "SMART_SCOPES_SERVER=";
    ::putenv(const_cast<char*>(server_url_env.c_str()));

    // reset url and check that we now have falback contant url
    EXPECT_NO_THROW(ssc_->reset_url());
    EXPECT_EQ("https://dash.ubuntu.com/smartscopes/v2", ssc_->url());

    // set the environment var
    server_url_env = "SMART_SCOPES_SERVER=http://hello.com/there";
    ::putenv(const_cast<char*>(server_url_env.c_str()));

    // reset url and check that we now have the environment var url
    EXPECT_NO_THROW(ssc_->reset_url());
    EXPECT_EQ("http://hello.com/there", ssc_->url());

    // force url
    EXPECT_NO_THROW(ssc_->reset_url("http://hello.com:2000/there"));
    EXPECT_EQ("http://hello.com:2000/there", ssc_->url());
}

} // namespace
