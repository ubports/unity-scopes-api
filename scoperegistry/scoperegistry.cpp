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
#include "SignalThread.h"

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
// same name, the OEM scope overrides the Canonical one.

map<string, string> find_local_scopes(string const& scope_installdir, string const& oem_installdir)
{
    // Look in scope_installdir for scope configuration files.
    // Scopes that do not permit themselves to be overridden are collected in fixed_scopes.
    // Scopes that can be overridden are collected in overrideable_scopes.
    // Each set contains file names (including the ".ini" suffix).

    map<string, string> fixed_scopes;           // Scopes that the OEM cannot override
    map<string, string> overrideable_scopes;    // Scopes that the OEM can override

    auto config_files = find_scope_config_files(scope_installdir, ".ini");
    for (auto path : config_files)
    {
        string file_name = basename(const_cast<char*>(string(path).c_str()));    // basename() modifies its argument
        string scope_name = strip_suffix(file_name, ".ini");
        try
        {
            ScopeConfig config(path);
            if (config.overrideable())
            {
                overrideable_scopes[scope_name] = path;
            }
            else
            {
                fixed_scopes[scope_name] = path;
            }
        }
        catch (unity::Exception const& e)
        {
            error("ignoring scope \"" + scope_name + "\": configuration error:\n" + e.what());
        }
    }

    map<string, string> oem_scopes;             // Additional scopes provided by the OEM (including overriding ones)
    if (!oem_installdir.empty())
    {
        auto oem_paths = find_scope_config_files(oem_installdir, ".ini");
        for (auto path : oem_paths)
        {
            string file_name = basename(const_cast<char*>(string(path).c_str()));    // basename() modifies its argument
            string scope_name = strip_suffix(file_name, ".ini");
            if (fixed_scopes.find(scope_name) == fixed_scopes.end())
            {
                overrideable_scopes.erase(scope_name);                // Only keep scopes that are not overridden by OEM
                oem_scopes[scope_name] = path;
            }
            else
            {
                error("ignoring non-overrideable scope config \"" + file_name + "\" in OEM directory " + oem_installdir);
            }
        }
    }

    // Combine fixed_scopes and overrideable scopes now.
    // overrideable_scopes only contains scopes that were *not* overridden by the OEM.
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
    for (auto pair : all_scopes)
    {
        try
        {
            unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(mw.get()));
            ScopeConfig sc(pair.second);
            mi->set_scope_name(pair.first);
            mi->set_display_name(sc.display_name());
            mi->set_description(sc.description());
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
            vector<string> spawn_command;
            spawn_command.push_back(scoperunner_path);
            spawn_command.push_back(config_file);
            spawn_command.push_back(pair.second);
            registry->add_local_scope(pair.first, move(meta), spawn_command);
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
    try
    {
        auto ss_reg = mw->create_registry_proxy(ss_reg_id, ss_reg_endpoint);
        registry->set_remote_scopes(ss_reg->list());
    }
    catch (MiddlewareException const& e)
    {
        // TODO: we need a refresh policy, to deal with re-started SS proxy,
        // as well as changes in the SS registry.
        error(e.what());
        error("cannot load remote scopes, skipping");
    }
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
        RuntimeImpl::UPtr runtime = RuntimeImpl::create("Registry", config_file);

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

        // The registry object stores the local and remote scopes
        RegistryObject::SPtr registry(new RegistryObject);

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
            string scope_name = strip_suffix(file_name, ".ini");
            local_scopes[scope_name] = argv[i];                   // operator[] overwrites pre-existing entries
            cerr << "extra: " << argv[i] << endl;
        }

        add_local_scopes(registry, local_scopes, middleware, scoperunner_path, config_file);
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

        // FIXME, HACK HACK HACK HACK
        // The middleware should spawn scope processes with lookup() on demand.
        // Because it currently does not have the plumbing, we start every scope immediately.
        // When the plumbing appears, remove this.
        for (auto pair : local_scopes)
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
        }

        // Wait until we are done (never, really, because the registry has no way to shut down other than
        // being killed by signal).
        middleware->wait_for_shutdown();
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
