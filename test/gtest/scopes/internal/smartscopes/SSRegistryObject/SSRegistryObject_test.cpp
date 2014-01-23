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

#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/scopes/ScopeExceptions.h>

#include "../RaiiServer.h"

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;
using namespace unity::test::scopes::internal::smartscopes;

TEST(SSRegistryObject, basic)
{
  RaiiServer test_server(FAKE_SSS_PATH);

  std::string server_env = "SMART_SCOPES_SERVER=http://127.0.0.1:" + std::to_string(test_server.port_);
  ::putenv((char*)server_env.c_str());

  RuntimeImpl::UPtr runtime = RuntimeImpl::create("SSRegistry", SS_RUNTIME_PATH);

  std::string identity = runtime->registry_identity();

  RegistryConfig c(identity, runtime->registry_configfile());
  std::string mw_kind = c.mw_kind();

  MiddlewareBase::SPtr middleware = runtime->factory()->find(identity, mw_kind);

  SSRegistryObject::SPtr reg(new SSRegistryObject(middleware));

  // list scopes
  MetadataMap scopes = reg->list();
  EXPECT_EQ(scopes.size(), 1);

  // visible scope
  ScopeMetadata meta = reg->get_metadata("Dummy Demo Scope");
  EXPECT_EQ("Dummy Demo Scope",meta.scope_name());
  EXPECT_EQ("Dummy Demo Scope",meta.display_name());
  EXPECT_EQ("Dummy demo scope.", meta.description());

  // invisible scope
  EXPECT_THROW(reg->get_metadata("Dummy Demo Scope 2"), NotFoundException);
}
