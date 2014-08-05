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

#include <unity/scopes/internal/smartscopes/HttpClientQt.h>
#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/scopes/internal/smartscopes/SmartScopesClient.h>

#include <unity/UnityExceptions.h>

#include "../RaiiServer.h"

#include <gtest/gtest.h>
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
        : http_client_(new HttpClientQt(20000)),
          json_node_(new JsonCppNode()),
          server_(FAKE_SSS_PATH)
    {
        sss_url_ = "http://127.0.0.1:" + std::to_string(server_.port_);
        ssc_ = std::make_shared<SmartScopesClient>(http_client_, json_node_, sss_url_);
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
    ASSERT_EQ(0, scopes.size());

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
    EXPECT_EQ(nullptr, scopes[0].appearance);
    EXPECT_EQ(nullptr, scopes[0].settings);

    EXPECT_EQ("dummy.scope.2", scopes[1].id);
    EXPECT_EQ("Dummy Demo Scope 2", scopes[1].name);
    EXPECT_EQ("Dummy demo scope 2.", scopes[1].description);
    EXPECT_EQ("Mr.Fake", scopes[1].author);
    EXPECT_EQ(sss_url_ + "/demo2", scopes[1].base_url);
    EXPECT_EQ(nullptr, scopes[1].icon);
    EXPECT_EQ("art", *scopes[1].art);
    EXPECT_TRUE(scopes[1].invisible);
    EXPECT_EQ("#00BEEF", (*scopes[1].appearance)["background"].get_string());
    EXPECT_EQ("logo.png", (*scopes[1].appearance)["PageHeader"].get_dict()["logo"].get_string());
    EXPECT_EQ(nullptr, scopes[1].settings);

    EXPECT_EQ("dummy.scope.3", scopes[2].id);
    EXPECT_EQ("Dummy Demo Scope 3", scopes[2].name);
    EXPECT_EQ("Dummy demo scope 3.", scopes[2].description);
    EXPECT_EQ("Mr.Fake", scopes[2].author);
    EXPECT_EQ(sss_url_ + "/demo3", scopes[2].base_url);
    EXPECT_EQ(nullptr, scopes[2].icon);
    EXPECT_EQ(nullptr, scopes[2].art);
    EXPECT_FALSE(scopes[2].invisible);
    EXPECT_EQ(nullptr, scopes[2].appearance);
    EXPECT_EQ("[{\"displayName\":\"Location\",\"id\":\"location\",\"parameters\":{\"defaultValue\":"
              "\"London\"},\"type\":\"string\"},{\"displayName\":\"Temperature Units\",\"id\":"
              "\"unitTemp\",\"parameters\":{\"defaultValue\":1,\"values\":[\"Celsius\",\"Fahrenheit"
              "\"]},\"type\":\"list\"},{\"displayName\":\"Age\",\"id\":\"age\",\"parameters\":{"
              "\"defaultValue\":23},\"type\":\"number\"},{\"displayName\":\"Enabled\",\"id\":"
              "\"enabled\",\"parameters\":{\"defaultValue\":true},\"type\":\"boolean\"}]\n",
              *scopes[2].settings);
}

TEST_F(SmartScopesClientTest, search)
{
    auto search_handle = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");

    auto search_results = search_handle->get_search_results();
    std::vector<SearchResult> results = std::get<3>(search_results);
    ASSERT_EQ(3u, results.size());

    EXPECT_EQ("URI", results[0].uri);
    EXPECT_EQ(nullptr, results[0].other_params["dnd_uri"]);
    EXPECT_EQ("Stuff", results[0].other_params["title"]->as_string());
    EXPECT_EQ(nullptr, results[0].other_params["icon"]);
    EXPECT_EQ("https://dash.ubuntu.com/imgs/amazon.png", results[0].other_params["art"]->as_string());
    EXPECT_EQ("cat1", results[0].category->id);
    EXPECT_EQ("Category 1", results[0].category->title);
    EXPECT_EQ("", results[0].category->icon);
    EXPECT_EQ("{}", results[0].category->renderer_template);

    EXPECT_EQ("URI2", results[1].uri);
    EXPECT_EQ(nullptr, results[1].other_params["dnd_uri"]);
    EXPECT_EQ("Things", results[1].other_params["title"]->as_string());
    EXPECT_EQ("https://dash.ubuntu.com/imgs/google.png", results[1].other_params["icon"]->as_string());
    EXPECT_EQ(nullptr, results[1].other_params["art"]);
    EXPECT_EQ("cat1", results[1].category->id);
    EXPECT_EQ("Category 1", results[1].category->title);
    EXPECT_EQ("", results[1].category->icon);
    EXPECT_EQ("{}", results[1].category->renderer_template);

    EXPECT_EQ("URI3", results[2].uri);
    EXPECT_EQ(nullptr, results[2].other_params["dnd_uri"]);
    EXPECT_EQ("Category Fail", results[2].other_params["title"]->as_string());
    EXPECT_EQ(nullptr, results[2].other_params["icon"]);
    EXPECT_EQ("https://dash.ubuntu.com/imgs/cat_fail.png", results[2].other_params["art"]->as_string());
    EXPECT_EQ(nullptr, results[2].category);

    // check departments
    auto dept = std::get<0>(search_results);
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
}

TEST_F(SmartScopesClientTest, preview)
{
    auto preview_handle = ssc_->preview(sss_url_ + "/demo", "result", "session_id", "platform", 0);

    auto results = preview_handle->get_preview_results();
    PreviewHandle::Columns columns = results.first;
    PreviewHandle::Widgets widgets = results.second;

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

    EXPECT_EQ("{\"id\":\"widget_id_A\",\"text\":\"First widget.\",\"title\":\"Widget A\",\"type\":\"text\"}\n", widgets[0]);
    EXPECT_EQ("{\"id\":\"widget_id_B\",\"text\":\"Second widget.\",\"title\":\"Widget B\",\"type\":\"text\"}\n", widgets[1]);
    EXPECT_EQ("{\"id\":\"widget_id_C\",\"text\":\"Third widget.\",\"title\":\"Widget C\",\"type\":\"text\"}\n", widgets[2]);
}

TEST_F(SmartScopesClientTest, consecutive_searches)
{
    auto search_handle1 = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle2 = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle3 = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle4 = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
    auto search_handle5 = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");

    auto search_results = search_handle1->get_search_results();
    std::vector<SearchResult> results = std::get<3>(search_results);
    EXPECT_EQ(3u, results.size());

    search_results = search_handle2->get_search_results();
    results = std::get<3>(search_results);
    EXPECT_EQ(3u, results.size());

    search_results = search_handle3->get_search_results();
    results = std::get<3>(search_results);
    EXPECT_EQ(3u, results.size());

    search_results = search_handle4->get_search_results();
    results = std::get<3>(search_results);
    EXPECT_EQ(3u, results.size());

    search_results = search_handle5->get_search_results();
    results = std::get<3>(search_results);
    EXPECT_EQ(3u, results.size());
}

TEST_F(SmartScopesClientTest, consecutive_cancels)
{
    for (int i = 0; i < 50; ++i)
    {
        auto search_handle = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");
        search_handle->cancel_search();
        EXPECT_THROW(search_handle->get_search_results(), std::exception);
    }

    auto search_handle = ssc_->search(sss_url_ + "/demo", "stuff", "", "session_id", 0, "platform");

    auto search_results = search_handle->get_search_results();
    std::vector<SearchResult> results = std::get<3>(search_results);
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
