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
        : http_client_(new HttpClientQt(2)),
          json_node_(new JsonCppNode()),
          server_(FAKE_SSS_PATH)
    {
        ssc_ = std::make_shared<SmartScopesClient>(http_client_, json_node_, "http://127.0.0.1", server_.port_);
    }

protected:
    HttpClientInterface::SPtr http_client_;
    JsonNodeInterface::SPtr json_node_;
    SmartScopesClient::SPtr ssc_;
    RaiiServer server_;
};

TEST_F(SmartScopesClientTest, remote_scopes)
{
    std::vector<RemoteScope> scopes = ssc_->get_remote_scopes();

    ASSERT_EQ(2, scopes.size());

    EXPECT_EQ("Dummy Demo Scope", scopes[0].name);
    EXPECT_EQ("https://productsearch.ubuntu.com/smartscopes/v2/search/demo", scopes[0].search_url);
    EXPECT_FALSE(scopes[0].invisible);

    EXPECT_EQ("Dummy Demo Scope 2", scopes[1].name);
    EXPECT_EQ("https://productsearch.ubuntu.com/smartscopes/v2/search/demo2", scopes[1].search_url);
    EXPECT_EQ(true, scopes[1].invisible);
}

TEST_F(SmartScopesClientTest, search)
{
    auto search_handle = ssc_->search("http://127.0.0.1/smartscopes/v2/search/demo", "stuff", "1234", 0, "");

    std::vector<SearchResult> results = search_handle->get_search_results();
    ASSERT_EQ(2, results.size());

    EXPECT_EQ("URI", results[0].uri);
    EXPECT_EQ("Stuff", results[0].title);
    EXPECT_EQ("https://productsearch.ubuntu.com/imgs/amazon.png", results[0].art);
    EXPECT_EQ("", results[0].dnd_uri);
    EXPECT_EQ("cat1", results[0].category->id);
    EXPECT_EQ("Category 1", results[0].category->title);
    EXPECT_EQ("", results[0].category->icon);
    EXPECT_EQ("", results[0].category->renderer_template);

    EXPECT_EQ("URI2", results[1].uri);
    EXPECT_EQ("Things", results[1].title);
    EXPECT_EQ("https://productsearch.ubuntu.com/imgs/google.png", results[1].art);
    EXPECT_EQ("", results[1].dnd_uri);
    EXPECT_EQ("cat1", results[1].category->id);
    EXPECT_EQ("Category 1", results[0].category->title);
    EXPECT_EQ("", results[1].category->icon);
    EXPECT_EQ("", results[1].category->renderer_template);
}

TEST_F(SmartScopesClientTest, consecutive_searches)
{
    auto search_handle1 = ssc_->search("http://127.0.0.1/smartscopes/v2/search/demo", "stuff", "1234", 0, "");
    auto search_handle2 = ssc_->search("http://127.0.0.1/smartscopes/v2/search/demo", "stuff", "1234", 0, "");

    std::vector<SearchResult> results = search_handle1->get_search_results();
    EXPECT_EQ(2, results.size());

    results = search_handle2->get_search_results();
    EXPECT_EQ(0, results.size());
}

} // namespace
