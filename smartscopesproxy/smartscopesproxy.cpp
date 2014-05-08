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
#include <unity/scopes/internal/RuntimeConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/smartscopes/SSScopeObject.h>
#include <unity/scopes/internal/smartscopes/SSRegistryObject.h>

#include <unity/scopes/internal/DfltConfig.h>

#include <core/posix/signal.h>

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

struct SignalThreadWrapper
{
    std::shared_ptr<core::posix::SignalTrap> termination_trap;
    std::thread termination_trap_worker;

    SignalThreadWrapper() :
        termination_trap(core::posix::trap_signals_for_all_subsequent_threads(
            {
                core::posix::Signal::sig_int,
                core::posix::Signal::sig_hup,
                core::posix::Signal::sig_term
            })),
        termination_trap_worker([&]() { termination_trap->run(); })
    {
    }

    core::Signal<core::posix::Signal>& signal_raised()
    {
        return termination_trap->signal_raised();
    }

    ~SignalThreadWrapper()
    {
        termination_trap->stop();
        if (termination_trap_worker.joinable())
            termination_trap_worker.join();
    }
};

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
    std::string server_url_env;
    std::string config_file;

    // check for "upstart" as first arg
    if (argc > 1 && std::string(argv[1]) == "upstart")
    {
        sig_upstart = true;
    }
    else
    {
        // argv[1]: server_url_env
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
        SignalThreadWrapper signal_handler;

        RuntimeConfig rt_config(config_file);
        std::string ss_reg_id = rt_config.ss_registry_identity();

        SSConfig ss_config(rt_config.ss_configfile());
        std::string ss_scope_id = ss_config.scope_identity();

        // Instantiate SS registry and scopes runtimes
        RuntimeImpl::UPtr reg_rt = RuntimeImpl::create(ss_reg_id, config_file);
        RuntimeImpl::UPtr scope_rt = RuntimeImpl::create(ss_scope_id, config_file);

        // Get registry config
        RegistryConfig reg_conf(ss_reg_id, reg_rt->registry_configfile());
        std::string mw_kind = reg_conf.mw_kind();
        std::string mw_configfile = reg_conf.mw_configfile();

        // Get middleware handles from runtimes
        MiddlewareBase::SPtr reg_mw = reg_rt->factory()->find(ss_reg_id, mw_kind);
        MiddlewareBase::SPtr scope_mw = scope_rt->factory()->create(ss_scope_id, mw_kind, mw_configfile);

        signal_handler.signal_raised().connect([reg_mw, scope_mw](core::posix::Signal)
        {
            scope_mw->stop();
            reg_mw->stop();
        });

        // Instantiate a SS registry object
        SSRegistryObject::SPtr reg(new SSRegistryObject(reg_mw, ss_config, scope_mw->get_scope_endpoint()));

        // Instantiate a SS scope object
        SSScopeObject::UPtr scope(new SSScopeObject(ss_scope_id, scope_mw, reg));

        // Add objects to the middlewares
        reg_mw->add_registry_object(reg_rt->ss_registry_identity(), reg);
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
