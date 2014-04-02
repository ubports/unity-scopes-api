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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "FindFiles.h"

#include <unity/scopes/internal/MiddlewareFactory.h>
#include <unity/scopes/internal/MWRegistry.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RegistryObject.h>
#include <unity/scopes/internal/RuntimeConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeConfig.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>
#include <unity/util/ResourcePtr.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <libgen.h>
#include <unistd.h>

using namespace scoperegistry;
using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::util;

char const* prog_name;

namespace
{

void error(string const& msg)
{
    assert(!msg.empty());
    cerr << prog_name << ": " << msg << endl;
}

string strip_suffix(string const& s, string const& suffix)
{
    auto s_len = s.length();
    auto suffix_len = suffix.length();
    if (s_len >= suffix_len)
    {
        if (s.compare(s_len - suffix_len, suffix_len, suffix) == 0)
        {
            return string(s, 0, s_len - suffix_len);
        }
    }
    return s;
}

// Return a map of <scope, config_file> pairs for all scopes (Canonical and OEM scopes).
// If a Canonical scope is overrideable and the OEM has configured a scope with the
// same id, the OEM scope overrides the Canonical one.

map<string, string> find_local_scopes(string const& scope_installdir, string const& oem_installdir)
{
    // Look in scope_installdir for scope configuration files.
    // Scopes that do not permit themselves to be overridden are collected in fixed_scopes.
    // Scopes that can be overridden are collected in overrideable_scopes.
    // Each set contains file names (including the ".ini" suffix).

    map<string, string> fixed_scopes;           // Scopes that the OEM cannot override
    map<string, string> overrideable_scopes;    // Scopes that the OEM can override

    auto config_files = find_scope_config_files(scope_installdir, ".ini");
    for (auto&& path : config_files)
    {
        string file_name = basename(const_cast<char*>(string(path).c_str()));    // basename() modifies its argument
        string scope_id = strip_suffix(file_name, ".ini");
        try
        {
            ScopeConfig config(path);
            if (config.overrideable())
            {
                overrideable_scopes[scope_id] = path;
            }
            else
            {
                fixed_scopes[scope_id] = path;
            }
        }
        catch (unity::Exception const& e)
        {
            error("ignoring scope \"" + scope_id + "\": configuration error:\n" + e.what());
        }
    }

    if (!oem_installdir.empty())
    {
        try
        {
            auto oem_paths = find_scope_config_files(oem_installdir, ".ini");
            for (auto&& path : oem_paths)
            {
                string file_name = basename(const_cast<char*>(string(path).c_str()));    // basename() modifies its argument
                string scope_id = strip_suffix(file_name, ".ini");
                if (fixed_scopes.find(scope_id) == fixed_scopes.end())
                {
                    overrideable_scopes[scope_id] = path;  // Replaces scope if it was present already
                }
                else
                {
                    error("ignoring non-overrideable scope config \"" + file_name + "\" in OEM directory " + oem_installdir);
                }
            }
        }
        catch (ResourceException const& e)
        {
            error(e.what());
            error("could not open OEM installation directory, ignoring OEM scopes");
        }
    }

    // Combine fixed_scopes and overrideable_scopes now.
    fixed_scopes.insert(overrideable_scopes.begin(), overrideable_scopes.end());
    return fixed_scopes;
}

// For each scope, open the config file for the scope, create the metadata info from the config,
// and add an entry to the RegistryObject.

void add_local_scopes(RegistryObject::SPtr const& registry,
                      map<string, string> const& all_scopes,
                      MiddlewareBase::SPtr const& mw,
                      string const& scoperunner_path,
                      string const& config_file)
{
    for (auto&& pair : all_scopes)
    {
        try
        {
            unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(mw.get()));
            ScopeConfig sc(pair.second);
            mi->set_scope_id(pair.first);
            mi->set_display_name(sc.display_name());
            mi->set_description(sc.description());
            mi->set_author(sc.author());
            mi->set_invisible(sc.invisible());
            mi->set_appearance_attributes(sc.appearance_attributes());
            try
            {
                mi->set_art(sc.art());
            }
            catch (NotFoundException const&)
            {
            }
            try
            {
                mi->set_icon(sc.icon());
            }
            catch (NotFoundException const&)
            {
            }
            try
            {
                mi->set_search_hint(sc.search_hint());
            }
            catch (NotFoundException const&)
            {
            }
            try
            {
                mi->set_hot_key(sc.hot_key());
            }
            catch (NotFoundException const&)
            {
            }

            ScopeProxy proxy = ScopeImpl::create(mw->create_scope_proxy(pair.first), mw->runtime(), pair.first);
            mi->set_proxy(proxy);
            auto meta = ScopeMetadataImpl::create(move(mi));

            RegistryObject::ScopeExecData exec_data;
            exec_data.scope_id = pair.first;
            // get custom scope runner executable, if not set use default scoperunner
            exec_data.scoperunner_path = scoperunner_path;
            try
            {
                auto custom_exec = sc.scope_runner();
                if (custom_exec.empty())
                {
                    throw unity::InvalidArgumentException("Invalid scope runner executable for scope: " + pair.first);
                }
                exec_data.scoperunner_path = custom_exec;
            }
            catch (NotFoundException const&)
            {
            }
            exec_data.runtime_config = config_file;
            exec_data.scope_config = pair.second;

            registry->add_local_scope(pair.first, move(meta), exec_data);
        }
        catch (unity::Exception const& e)
        {
            error("ignoring scope \"" + pair.first + "\": cannot create metadata: " + e.what());
        }
    }
}

// Overwrite any remote scopes loaded previously with the current ones.

void load_remote_scopes(RegistryObject::SPtr const& registry,
                        MiddlewareBase::SPtr const& mw,
                        string const& ss_reg_id,
                        string const& ss_reg_endpoint)
{
    auto ss_reg = mw->create_registry_proxy(ss_reg_id, ss_reg_endpoint);
    registry->set_remote_registry(ss_reg);
}

} // namespace

// Usage: scoperegistry [runtime.ini] [scope.ini]...
//
// If no runtime config file is specified, the default location (/usr/share/unity-scopes-api/Runtime.ini)
// is assumed.
// If additional scope configuration files are specified, the corresponding scopes will be added
// to the registry (overriding any scopes that are found via config files reached via Runtime.ini).

void
usage(ostream& s = cerr)
{
    s << "usage: " << prog_name << " [runtime.ini] [scope.ini]..." << endl;
}

int
main(int argc, char* argv[])
{
    prog_name = basename(argv[0]);
    if (argc > 1 && (string("-h") == argv[1] || string("--help") == argv[1]))
    {
        usage(cout);
        exit(0);
    }

    char const* const config_file = argc > 1 ? argv[1] : "";
    int exit_status = 1;

    try
    {
        // We shutdown the runtime whenever these signals happen.
        auto termination_trap = core::posix::trap_signals_for_all_subsequent_threads(
        {
            core::posix::Signal::sig_int,
            core::posix::Signal::sig_hup,
            core::posix::Signal::sig_term
        });

        // And we maintain our list of processes with the help of sig_chld.
        auto child_trap = core::posix::trap_signals_for_all_subsequent_threads(
        {
            core::posix::Signal::sig_chld
        });

        // The death observer is required to make sure that we reap all child processes
        // whenever multiple sigchld's are compressed together.
        auto death_observer =
                core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(
                    child_trap);

        // Starting up both traps.
        std::thread termination_trap_worker([termination_trap]() { termination_trap->run(); });
        std::thread child_trap_worker([child_trap]() { child_trap->run(); });

        // And finally creating our runtime.
        RuntimeConfig rt_config(config_file);
        RuntimeImpl::UPtr runtime = RuntimeImpl::create(rt_config.registry_identity(), config_file);

        string identity = runtime->registry_identity();

        // Collect the registry config data.

        string mw_kind;
        string mw_endpoint;
        string mw_configfile;
        string scope_installdir;
        string oem_installdir;
        string scoperunner_path;
        string ss_reg_id;
        string ss_reg_endpoint;
        {
            RegistryConfig c(identity, runtime->registry_configfile());
            mw_kind = c.mw_kind();
            mw_endpoint = c.endpoint();
            mw_configfile = c.mw_configfile();
            scope_installdir = c.scope_installdir();
            oem_installdir = c.oem_installdir();
            scoperunner_path = c.scoperunner_path();
            ss_reg_id = c.ss_registry_identity();
            ss_reg_endpoint = c.ss_registry_endpoint();
        } // Release memory for config parser

        MiddlewareBase::SPtr middleware = runtime->factory()->find(identity, mw_kind);

        // Inform the signal thread that it should shutdown the middleware
        // if we get a termination signal.
        termination_trap->signal_raised().connect([middleware](core::posix::Signal signal)
        {
            switch(signal)
            {
            case core::posix::Signal::sig_int:
            case core::posix::Signal::sig_hup:
            case core::posix::Signal::sig_term:
                middleware->stop();
                break;
            default:
                break;
            }
        });

        // The registry object stores the local and remote scopes
        RegistryObject::SPtr registry(new RegistryObject(*death_observer));

        // Add the metadata for each scope to the lookup table.
        // We do this before starting any of the scopes, so aggregating scopes don't get a lookup failure if
        // they look for another scope in the registry.

        auto local_scopes = find_local_scopes(scope_installdir, oem_installdir);

        // Before we add the local scopes, we check whether any scopes were explicitly specified
        // on the command line. If so, scopes on the command line override scopes in
        // configuration files.
        for (auto i = 2; i < argc; ++i)
        {
            string file_name = basename(const_cast<char*>(string(argv[i]).c_str()));  // basename() modifies its argument
            string scope_id = strip_suffix(file_name, ".ini");
            local_scopes[scope_id] = argv[i];                   // operator[] overwrites pre-existing entries
        }

        add_local_scopes(registry, local_scopes, middleware, scoperunner_path, runtime->configfile());
        if (ss_reg_id.empty() || ss_reg_endpoint.empty())
        {
            error("no remote registry configured, only local scopes will be available");
        }
        else
        {
            load_remote_scopes(registry, middleware, ss_reg_id, ss_reg_endpoint);
        }

        // Now that the registry table is populated, we can add the registry to the middleware, so
        // it starts processing incoming requests.
        middleware->add_registry_object(runtime->registry_identity(), registry);

        // We also add the registry's state receiver to the middleware so that scopes can inform
        // the registry of state changes.
        middleware->add_state_receiver_object("StateReceiver", registry->state_receiver());

        // FIXME, HACK HACK HACK HACK
        // The middleware should spawn scope processes with lookup() on demand.
        // Because it currently does not have the plumbing, we start every scope immediately.
        // When the plumbing appears, remove this.
        for (auto&& pair : local_scopes)
        {
            try
            {
                registry->locate(pair.first);
            }
            catch (NotFoundException const&)
            {
                // We ignore this. If the scope config couldn't be found, add_local_scopes()
                // has already printed an error message.
            }
            catch (std::exception const& e)
            {
                error("could not start scope " + pair.first + ": " + e.what());
            }
        }

        // Drop our shared_ptr to the RegistryObject. This means that the registry object
        // is kept alive only via the shared_ptr held by the middleware. If the middleware
        // shuts down, it clears out the active servant map, which destroys the registry
        // object. The registry object kills all its child processes as part of its clean-up.
        registry = nullptr;

        // Wait until we are done, which happens if we receive a termination signal.
        middleware->wait_for_shutdown();

        // Stop termination_trap
        termination_trap->stop();
        if (termination_trap_worker.joinable())
            termination_trap_worker.join();

        // Please note that the child_trap must only be stopped once the
        // termination_trap thread has been joined. We otherwise will encounter
        // a race between the middleware shutting down and not receiving sigchld anymore.
        child_trap->stop();
        if (child_trap_worker.joinable())
            child_trap_worker.join();

        exit_status = 0;
    }
    catch (std::exception const& e)
    {
        error(e.what());
    }
    catch (string const& e)
    {
        error("fatal error: " + e);
    }
    catch (char const* e)
    {
        error(string("fatal error: ") + e);
    }
    catch (...)
    {
        error("terminated due to unknown exception");
    }

    return exit_status;
}
