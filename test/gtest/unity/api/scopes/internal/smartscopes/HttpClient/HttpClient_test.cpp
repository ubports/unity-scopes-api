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
    system("killall -q BadServer.py");
    system("killall -q GoodServer.py");
  }

protected:
  HttpClientInterface::SPtr http_client_;
};

TEST_F( HttpClientTest, bad_server )
{
  // no server

  std::future< std::string > response = http_client_->get( test_url, test_port );
  response.wait();

  EXPECT_THROW( response.get(), std::runtime_error );

  // bad server

  system("./BadServer.py &");
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

  response = http_client_->get( test_url, test_port );
  response.wait();

  EXPECT_THROW( response.get(), std::runtime_error );

  system("killall -q BadServer.py");
}

TEST_F( HttpClientTest, good_server )
{
  // responds immediately

  system("./GoodServer.py 0 &");
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

  std::future< std::string > response = http_client_->get( test_url, test_port );
  response.wait();

  std::string response_str;
  EXPECT_NO_THROW( response_str = response.get() );
  EXPECT_EQ( "Hello there", response_str );

  system("killall -q GoodServer.py");

  // responds in 1 second

  system("./GoodServer.py 1 &");
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

  response = http_client_->get( test_url, test_port );
  response.wait();

  EXPECT_NO_THROW( response_str = response.get() );
  EXPECT_EQ( "Hello there", response_str );

  system("killall -q GoodServer.py");
}

TEST_F( HttpClientTest, slow_server )
{
  // responds in 5 seconds

  system("./GoodServer.py 5 &");
  std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

  std::future< std::string > response = http_client_->get( test_url, test_port );
  response.wait();

  EXPECT_THROW( response.get(), std::runtime_error );

  system("killall -q GoodServer.py");
}

} // namespace
