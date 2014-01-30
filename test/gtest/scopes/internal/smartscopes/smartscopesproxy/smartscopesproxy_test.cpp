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

using namespace testing;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;
using namespace unity::test::scopes::internal::smartscopes;

///! TODO: more tests

namespace
{

class smartscopesproxytest : public Test
{
public:
    smartscopesproxytest()
        : server_(FAKE_SSS_PATH)
    {
        std::string server_env = "SMART_SCOPES_SERVER=http://127.0.0.1:" + std::to_string(server_.port_);
        ::putenv((char*)server_env.c_str());

        // Instantiate SS registry and scopes runtimes
        reg_rt_ = RuntimeImpl::create("SSRegistry", SS_RUNTIME_PATH);
        scope_rt_ = RuntimeImpl::create("SmartScope", SS_RUNTIME_PATH);

        // Get registry config
        RegistryConfig reg_conf("SSRegistry", reg_rt_->registry_configfile());
        std::string mw_kind = reg_conf.mw_kind();
        std::string mw_configfile = reg_conf.mw_configfile();

        // Get middleware handles from runtimes
        reg_mw_ = reg_rt_->factory()->find("SSRegistry", mw_kind);
        scope_mw_ = scope_rt_->factory()->create("SmartScope", mw_kind, mw_configfile);

        // Instantiate a SS registry and scope objects
        reg_ = SSRegistryObject::SPtr(new SSRegistryObject(reg_mw_, scope_mw_->get_scope_endpoint(), 2, 2000, 60));
        scope_ = SSScopeObject::UPtr(new SSScopeObject("SmartScope", scope_mw_, reg_));

        // Add objects to the middlewares
        reg_mw_->add_registry_object(reg_rt_->registry_identity(), reg_);
        scope_mw_->add_dflt_scope_object(std::move(scope_));
    }

    ~smartscopesproxytest()
    {
        scope_mw_->stop();
        scope_mw_->wait_for_shutdown();

        reg_mw_->stop();
        reg_mw_->wait_for_shutdown();
    }

protected:
    RuntimeImpl::UPtr reg_rt_;
    RuntimeImpl::UPtr scope_rt_;

    MiddlewareBase::SPtr reg_mw_;
    MiddlewareBase::SPtr scope_mw_;

    SSRegistryObject::SPtr reg_;
    SSScopeObject::UPtr scope_;

    RaiiServer server_;
};

TEST_F(smartscopesproxytest, basic)
{
    // list scopes
    MetadataMap scopes = reg_->list();
    EXPECT_EQ(scopes.size(), 1);

    // visible scope
    ScopeMetadata meta = reg_->get_metadata("Dummy Demo Scope");
    EXPECT_EQ("Dummy Demo Scope", meta.scope_name());
    EXPECT_EQ("Dummy Demo Scope", meta.display_name());
    EXPECT_EQ("Dummy demo scope.", meta.description());

    // invisible scope
    EXPECT_THROW(reg_->get_metadata("Dummy Demo Scope 2"), NotFoundException);
}

} // namespace
