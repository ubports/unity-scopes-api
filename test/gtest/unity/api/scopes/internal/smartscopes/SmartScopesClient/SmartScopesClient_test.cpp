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

using namespace testing;
using namespace unity::api::scopes::internal::smartscopes;

namespace
{

class SmartScopesClientTest : public Test
{
public:
  SmartScopesClientTest()
    : http_client_( new HttpClientQt() ),
      json_node_( new JsonCppNode() ),
      ssc_( http_client_, json_node_, "http://127.0.0.1/", 9009 ) {}

protected:
  HttpClientInterface::SPtr http_client_;
  JsonNodeInterface::SPtr json_node_;
  SmartScopesClient ssc_;
};

TEST_F( SmartScopesClientTest, basic )
{
  system("./FakeSss.py &");

  std::vector< RemoteScope > scopes = ssc_.get_remote_scopes();
  scopes = ssc_.get_remote_scopes();
}

} // namespace
