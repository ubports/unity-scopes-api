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

#include <gtest/gtest.h>
#include <memory>
#include <thread>

using namespace testing;
using namespace unity::api::scopes::internal::smartscopes;

namespace
{

const std::string test_url = "http://127.0.0.1";
int test_port = 9009;

class HttpClientTest : public Test
{
public:
    HttpClientTest()
        : http_client_( new HttpClientQt() )
    {
    }

    void run_server( const std::string& server_path, const std::string& arg = "0" )
    {
        const char* const argv[] = {server_path.c_str(), arg.c_str(), NULL};

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
    pid_t pid_;
};

TEST_F( HttpClientTest, bad_server )
{
    // no server

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    EXPECT_THROW( response.get(), std::runtime_error );

    // bad server

    run_server( BAD_SERVER_PATH );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    response = http_client_->get( test_url, test_port );
    response.wait();

    EXPECT_THROW( response.get(), std::runtime_error );

    kill_server();
}

TEST_F( HttpClientTest, good_server )
{
    // responds immediately

    run_server( GOOD_SERVER_PATH );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    std::string response_str;
    EXPECT_NO_THROW( response_str = response.get() );
    EXPECT_EQ( "Hello there", response_str );

    kill_server();

    // responds in 1 second

    run_server( GOOD_SERVER_PATH, "1" );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    response = http_client_->get( test_url, test_port );
    response.wait();

    EXPECT_NO_THROW( response_str = response.get() );
    EXPECT_EQ( "Hello there", response_str );

    kill_server();
}

TEST_F( HttpClientTest, slow_server )
{
    // responds in 5 seconds

    run_server( GOOD_SERVER_PATH, "5" );
    std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    EXPECT_THROW( response.get(), std::runtime_error );

    kill_server();
}

} // namespace
