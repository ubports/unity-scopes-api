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
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <memory>
#include <thread>

using namespace testing;
using namespace unity::api::scopes::internal::smartscopes;

namespace
{

const std::string test_url = "http://127.0.0.1";
int test_port = 9008;

class HttpClientTest : public Test
{
public:
    HttpClientTest()
        : http_client_( new HttpClientQt() )
    {
    }

    class server_raii
    {
    public:
        server_raii( const std::string& server_path, const std::string& arg = "0" )
        {
            const char* const argv[] = {server_path.c_str(), arg.c_str(), NULL};

            switch (pid_ = fork())
            {
                case -1:
                    throw unity::ResourceException("Failed to fork process");
                case 0: // child
                    execv(argv[0], (char *const*)argv);
                    throw unity::ResourceException("Failed to fork process");
            }

            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        }

        ~server_raii()
        {
            kill( pid_, SIGKILL );
            std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        }

    private:
        pid_t pid_ = -1;
    };

protected:
    HttpClientInterface::SPtr http_client_;
};

TEST_F( HttpClientTest, no_server )
{
    // no server

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    EXPECT_THROW( response.get(), unity::Exception );
}

TEST_F( HttpClientTest, bad_server )
{
    // bad server

    server_raii server( BAD_SERVER_PATH );

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    EXPECT_THROW( response.get(), unity::Exception );
}

TEST_F( HttpClientTest, good_server )
{
    // responds immediately

    server_raii server( GOOD_SERVER_PATH );

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    std::string response_str;
    EXPECT_NO_THROW( response_str = response.get() );
    EXPECT_EQ( "Hello there", response_str );
}

TEST_F( HttpClientTest, ok_server )
{
    // responds in 1 second

    server_raii server( GOOD_SERVER_PATH, "1" );

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    std::string response_str;
    EXPECT_NO_THROW( response_str = response.get() );
    EXPECT_EQ( "Hello there", response_str );
}

TEST_F( HttpClientTest, slow_server )
{
    // responds in 5 seconds

    server_raii server( GOOD_SERVER_PATH, "5" );

    std::future<std::string> response = http_client_->get( test_url, test_port );
    response.wait();

    EXPECT_THROW( response.get(), unity::Exception );
}

TEST_F( HttpClientTest, percent_encoding )
{
    std::string encoded_str = http_client_->to_percent_encoding( " \"%<>\\^`{|}!*'();:@&=+$,/?#[]" );
    EXPECT_EQ( "%20%22%25%3C%3E%5C%5E%60%7B%7C%7D%21%2A%27%28%29%3B%3A%40%26%3D%2B%24%2C%2F%3F%23%5B%5D", encoded_str );
}

} // namespace
