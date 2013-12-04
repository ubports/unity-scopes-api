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

using namespace testing;
using namespace unity::api::scopes::internal::smartscopes;

namespace
{

const std::string test_url = "https://productsearch.ubuntu.com/smartscopes/v1/search?query=hello";

class HttpClientTest : public Test
{
public:
  HttpClientTest()
    : http_client_( new HttpClientQt() ),
      http_client2_( new HttpClientQt() ) {}

protected:
  HttpClientInterface::SPtr http_client_;
  HttpClientInterface::SPtr http_client2_;
};

TEST_F( HttpClientTest, basic )
{
  std::future< std::string > r = http_client_->get( test_url );
  r.wait();

  r = http_client_->get( test_url );
  r.wait();

  r = http_client2_->get( test_url );
  r.wait();

  std::string reply_string = r.get();
  std::cout << reply_string;
}

} // namespace
