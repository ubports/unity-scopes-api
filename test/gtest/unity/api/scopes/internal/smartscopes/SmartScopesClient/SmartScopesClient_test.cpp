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

#include <scopes/internal/smartscopes/HttpClientQt.h>
#include <scopes/internal/smartscopes/JsonCppNode.h>
#include <scopes/internal/smartscopes/SmartScopesClient.h>

#include <gtest/gtest.h>
#include <memory>
#include <thread>

using namespace testing;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal::smartscopes;

namespace
{

class SmartScopesClientTest : public Test
{
public:
    SmartScopesClientTest()
        : http_client_( new HttpClientQt() ),
          json_node_( new JsonCppNode() ),
          ssc_( http_client_, json_node_, "http://127.0.0.1", 9009 )
    {
    }

    ~SmartScopesClientTest()
    {
        kill_server();
    }

    void run_server()
    {
        const char* const argv[] = {FAKE_SSS_PATH, "", NULL};

        switch (pid_ = fork())
        {
            case -1:
                FAIL();
            case 0: // child
                execv(argv[0], (char *const*)argv);
                FAIL();
        }
    }

    void kill_server()
    {
        kill( pid_, SIGKILL );
    }

protected:
    HttpClientInterface::SPtr http_client_;
    JsonNodeInterface::SPtr json_node_;
    SmartScopesClient ssc_;

    pid_t pid_;
};

TEST_F( SmartScopesClientTest, remote_scopes )
{
    run_server();

    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    std::vector<RemoteScope> scopes = ssc_.get_remote_scopes();

    ASSERT_EQ( 2, scopes.size() );

    EXPECT_EQ( "Dummy Demo Scope", scopes[0].name );
    EXPECT_EQ( "https://productsearch.ubuntu.com/smartscopes/v2/search/demo", scopes[0].search_url );
    EXPECT_EQ( false, scopes[0].invisible );

    EXPECT_EQ( "Dummy Demo Scope 2", scopes[1].name );
    EXPECT_EQ( "https://productsearch.ubuntu.com/smartscopes/v2/search/demo2", scopes[1].search_url );
    EXPECT_EQ( true, scopes[1].invisible );

    kill_server();
}

TEST_F( SmartScopesClientTest, search )
{
    run_server();

    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    ssc_.search( "/smartscopes/v2/search/demo", "stuff", "1234", 0, "" );
    std::vector<SearchResult> results = ssc_.get_search_results();

    ASSERT_EQ( 2, results.size() );

    EXPECT_EQ( "URI", results[0].uri );
    EXPECT_EQ( "Stuff", results[0].title );
    EXPECT_EQ( "https://productsearch.ubuntu.com/imgs/amazon.png", results[0].art );
    EXPECT_EQ( "", results[0].dnd_uri );
    EXPECT_EQ( "cat1", results[0].category->id );
    EXPECT_EQ( "Category 1", results[0].category->title );
    EXPECT_EQ( "", results[0].category->icon );
    EXPECT_EQ( "", results[0].category->renderer_template );

    EXPECT_EQ( "URI2", results[1].uri );
    EXPECT_EQ( "Things", results[1].title );
    EXPECT_EQ( "https://productsearch.ubuntu.com/imgs/google.png", results[1].art );
    EXPECT_EQ( "", results[1].dnd_uri );
    EXPECT_EQ( "cat1", results[1].category->id );
    EXPECT_EQ( "Category 1", results[0].category->title );
    EXPECT_EQ( "", results[1].category->icon );
    EXPECT_EQ( "", results[1].category->renderer_template );

    kill_server();
}

} // namespace
