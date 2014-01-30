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
#include <unity/scopes/internal/smartscopes/SSScopeObject.h>
#include <unity/scopes/ScopeExceptions.h>

#include "../RaiiServer.h"

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;
using namespace unity::test::scopes::internal::smartscopes;

///! TODO: more tests

TEST(smartscopesproxy, basic)
{
    RaiiServer test_server(FAKE_SSS_PATH);

    std::string server_env = "SMART_SCOPES_SERVER=http://127.0.0.1:" + std::to_string(test_server.port_);
    ::putenv((char*)server_env.c_str());

    // Instantiate SS registry and scopes runtimes
    RuntimeImpl::UPtr reg_rt = RuntimeImpl::create("SSRegistry", SS_RUNTIME_PATH);
    RuntimeImpl::UPtr scope_rt = RuntimeImpl::create("SmartScope", SS_RUNTIME_PATH);

    // Get registry config
    RegistryConfig reg_conf("SSRegistry", reg_rt->registry_configfile());
    std::string mw_kind = reg_conf.mw_kind();
    std::string mw_configfile = reg_conf.mw_configfile();

    // Get middleware handles from runtimes
    MiddlewareBase::SPtr reg_mw = reg_rt->factory()->find("SSRegistry", mw_kind);
    MiddlewareBase::SPtr scope_mw = scope_rt->factory()->create("SmartScope", mw_kind, mw_configfile);

    // Instantiate a SS registry object
    SSRegistryObject::SPtr reg(new SSRegistryObject(reg_mw, scope_mw->get_scope_endpoint(),
                                                    2, 2000, 60));

    // Instantiate a SS scope object
    SSScopeObject::UPtr scope(new SSScopeObject("SmartScope", scope_mw, reg));

    // Add objects to the middlewares
    reg_mw->add_registry_object(reg_rt->registry_identity(), reg);
    scope_mw->add_dflt_scope_object(std::move(scope));

    // list scopes
    MetadataMap scopes = reg->list();
    EXPECT_EQ(scopes.size(), 1);

    // visible scope
    ScopeMetadata meta = reg->get_metadata("Dummy Demo Scope");
    EXPECT_EQ("Dummy Demo Scope", meta.scope_name());
    EXPECT_EQ("Dummy Demo Scope", meta.display_name());
    EXPECT_EQ("Dummy demo scope.", meta.description());

    // invisible scope
    EXPECT_THROW(reg->get_metadata("Dummy Demo Scope 2"), NotFoundException);

    scope_mw->stop();
    scope_mw->wait_for_shutdown();

    reg_mw->stop();
    reg_mw->wait_for_shutdown();
}
