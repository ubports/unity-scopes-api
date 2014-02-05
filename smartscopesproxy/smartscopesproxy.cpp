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

#include <unity/scopes/internal/DfltConfig.h>

#include <cassert>
#include <signal.h>
#include <libgen.h>
#include <iostream>
#include <unistd.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::internal::smartscopes;

static void error(std::string const& msg)
{
    assert(!msg.empty());
    std::cerr << "smartscopesproxy: " << msg << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc > 1 && (std::string("-h") == argv[1] || std::string("--help") == argv[1]))
    {
        char const* prog_name = basename(argv[0]);
        std::cout << "usage: " << prog_name << " [sss_url] [runtime.ini]" << std::endl;
        return 0;
    }

    int exit_status = 1;

    bool sig_upstart = false;
    std::string config_file;

    // check for "upstart" as first arg
    if (argc > 1 && std::string(argv[1]) == "upstart")
    {
        sig_upstart = true;
    }
    else
    {
        // argv[1]: server_url_env
        std::string server_url_env;
        if (argc > 1)
        {
            server_url_env = "SMART_SCOPES_SERVER=" + std::string(argv[1]);
            ::putenv(const_cast<char*>(server_url_env.c_str()));
        }

        // argv[2]: config_file
        config_file = argc > 2 ? argv[2] : "";
    }

    try
    {
        ///! TODO: get these from config
        std::string ss_reg_id = "SSRegistry";
        std::string ss_scope_id = "SmartScope";
        uint const max_sessions = 4;
        uint const no_reply_timeout = 5000;
        uint const ss_reg_refresh_rate = 60 * 24; // 24 hour refresh

        // Instantiate SS registry and scopes runtimes
        RuntimeImpl::UPtr reg_rt = RuntimeImpl::create(ss_reg_id, DFLT_SS_RUNTIME_INI);
        RuntimeImpl::UPtr scope_rt = RuntimeImpl::create(ss_scope_id, config_file);

        // Get registry config
        RegistryConfig reg_conf(ss_reg_id, reg_rt->registry_configfile());
        std::string mw_kind = reg_conf.mw_kind();
        std::string mw_configfile = reg_conf.mw_configfile();

        // Get middleware handles from runtimes
        MiddlewareBase::SPtr reg_mw = reg_rt->factory()->find(ss_reg_id, mw_kind);
        MiddlewareBase::SPtr scope_mw = scope_rt->factory()->create(ss_scope_id, mw_kind, mw_configfile);

        // Instantiate a SS registry object
        SSRegistryObject::SPtr reg(new SSRegistryObject(reg_mw, scope_mw->get_scope_endpoint(), max_sessions,
                                                        no_reply_timeout, ss_reg_refresh_rate));

        // Instantiate a SS scope object
        SSScopeObject::UPtr scope(new SSScopeObject(ss_scope_id, scope_mw, reg));

        // Add objects to the middlewares
        reg_mw->add_registry_object(reg_rt->registry_identity(), reg);
        scope_mw->add_dflt_scope_object(std::move(scope));

        if (sig_upstart)
        {
            // signal to upstart that we are ready
            kill(getpid(), SIGSTOP);
        }

        // Wait until shutdown
        scope_mw->wait_for_shutdown();
        reg_mw->wait_for_shutdown();

        exit_status = 0;
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
