/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
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
#include <unity/scopes/internal/smartscopes/SSScopeObject.h>
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream>

using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;

static void error(std::string const& msg)
{
    assert(!msg.empty());
    std::cerr << "smartscopesproxy: " << msg << std::endl;
}

int main(int argc, char* argv[])
{
    int exit_status = 1;

    std::string server_url_env;
    if (argc > 1)
    {
        server_url_env = "SMART_SCOPES_SERVER=" + std::string(argv[1]);
        ::putenv(const_cast<char*>(server_url_env.c_str()));
    }

    try
    {
        std::string reg_id = "SSRegistry";
        std::string scope_id = "SmartScope";

        // SMART SCOPES REGISTRY
        // =====================

        // Instantiate a runtime
        RuntimeImpl::UPtr reg_rt = RuntimeImpl::create(reg_id, SS_RUNTIME_PATH);

        // Get registry config
        RegistryConfig reg_conf(reg_id, reg_rt->registry_configfile());
        std::string mw_kind = reg_conf.mw_kind();
        std::string mw_configfile = reg_conf.mw_configfile();

        // Get registry middleware handle from runtime
        MiddlewareBase::SPtr reg_mw = reg_rt->factory()->find(reg_id, mw_kind);

        // Instantiate a SS registry object
        SSRegistryObject::SPtr registry(new SSRegistryObject(reg_mw, scope_id));

        // Add the SS registry object to the middleware
        reg_mw->add_registry_object(reg_rt->registry_identity(), registry);

        // SMART SCOPES SCOPE
        // ==================

        // Instantiate a runtime
        RuntimeImpl::UPtr scope_rt = RuntimeImpl::create(scope_id, SS_RUNTIME_PATH);

        // Get scope middleware handle from runtime
        MiddlewareBase::SPtr scope_mw = scope_rt->factory()->create(scope_id, mw_kind, mw_configfile);

        // Instantiate a SS scope object
        SSScopeObject::UPtr scope = SSScopeObject::UPtr(new SSScopeObject(scope_mw, registry));

        // Add the SS scope object to the middleware
        scope_mw->add_dflt_scope_object(std::move(scope));

        // Wait until shutdown
        scope_mw->wait_for_shutdown();
        reg_mw->wait_for_shutdown();

        exit_status = 0;
    }
    catch (unity::Exception const& e)
    {
        error(e.to_string());
    }
    catch (std::exception const& e)
    {
        error(e.what());
    }
    catch (std::string const& e)
    {
        error("fatal error: " + e);
    }
    catch (char const* e)
    {
        error(std::string("fatal error: ") + e);
    }
    catch (...)
    {
        error("terminated due to unknown exception");
    }

    return exit_status;
}
