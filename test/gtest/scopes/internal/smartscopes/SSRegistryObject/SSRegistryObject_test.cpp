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
//#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;

TEST(SSRegistryObject, basic)
{
  char server_url[] = "SMART_SCOPES_SERVER=http://127.0.0.1:8000";
  ::putenv(server_url);

  RuntimeImpl::UPtr runtime = RuntimeImpl::create("SSRegistry", SS_RUNTIME_PATH);

  std::string identity = runtime->registry_identity();

  RegistryConfig c(identity, runtime->registry_configfile());
  std::string mw_kind = c.mw_kind();

  MiddlewareBase::SPtr middleware = runtime->factory()->find(identity, mw_kind);

  SSRegistryObject::SPtr reg(new SSRegistryObject(middleware));

//  ScopeMetadata meta = reg->get_metadata("Wikipedia");
//  EXPECT_EQ("Wikipedia",meta.scope_name());
//  EXPECT_EQ("Wikipedia",meta.display_name());
//  EXPECT_EQ("The free encyclopedia that anyone can edit.", meta.description());
}
