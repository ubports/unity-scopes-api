/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include "FindFiles.h"
#include "SignalThread.h"

#include <scopes/internal/MiddlewareFactory.h>
#include <scopes/internal/RegistryConfig.h>
#include <scopes/internal/RegistryObject.h>
#include <scopes/internal/RuntimeConfig.h>
#include <scopes/internal/RuntimeImpl.h>
#include <scopes/internal/ScopeConfig.h>
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
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;
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

// Scan group config dir for .ini files. Each file is assumed to define a single scope group, in a group with key "ScopeGroup".
// The key "Scopes" in the group is expected to contain an array of scope names.
// For each scope group, the returned vector contains a map with the scope and .ini file names for that group.
// If a particular scope appears in more than one group file, the first file found determines which group the scope
// belongs too. (Subsequent mentions of a scope already in a group print a warning.)
// all_scopes must be the map of all scopes that were originally found in config dir.
// For any scopes not in a group, the returned vector contains a map containing just that scope. In other words,
// the returned vector contains as many maps as there will be scoperunner processes, with each map containing
// the scope name(s) and scope config file(s) for a process.

vector<map<string, string>> create_scope_groups(string const& group_dir, map<string, string> all_scopes)
{
    set<string> scopes_seen;                     // Names of scopes that are in a group so far
    vector<map<string, string>> scope_groups;    // One map per scope group
    if (!group_dir.empty())
    {
        auto group_files = find_files(group_dir, ".ini");
        for (auto file : group_files)
        {
            IniParser::SPtr parser;
            try
            {
                parser = make_shared<IniParser>(file.c_str());
            }
            catch (FileException const& e)
            {
                error(e.to_string() + ": scope group file ignored");
                continue;
            }

            vector<string> scopes;
            try
            {
                scopes = parser->get_string_array("ScopeGroup", "Scopes");
            }
            catch (LogicException const& e)
            {
                error("group file \"" + file + ": file ignored: " + e.to_string());
                continue;
            }

            // For each scope name in the group, push an element onto the vector of scope groups, but only if we have
            // not seen that scope name in an earlier group.
            bool once = false;
            for (auto scope : scopes)
            {
                if (scopes_seen.find(scope) != scopes_seen.end())
                {
                    error("ignoring scope \"" + scope + "\" in group file " + file + ": scope is already part of a group");
                    continue;
                }
                auto it = all_scopes.find(scope);
                if (it == all_scopes.end())
                {
                    error("ignoring scope \"" + scope + "\" in group file " + file + ": cannot find configuration for this scope");
                    continue;
                }
                scopes_seen.insert(scope);
                if (!once)
                {
                    once = true;
                    scope_groups.push_back(map<string, string>());
                }
                scope_groups.rbegin()->insert(make_pair(scope, it->second));
                all_scopes.erase(it);                // Any scope in a group is removed from all_scopes.
            }
        }
    }

    // Any scopes remaining in all_scopes at this point were not part of a group and therefore are in a map by themselves.
    for (auto pair : all_scopes)
    {
        map<string, string> s;
        s.insert(pair);
        scope_groups.push_back(s);
    }

    return scope_groups;
}

void run_scopes(SignalThread& sigthread,
                string const& scoperunner_path,
                string const& config_file,
                vector<map<string, string>> const& groups)
{
    // Cobble together an argv for each scope group so we can fork/exec the scope runner for the group.
    for (auto group : groups)
    {
        unique_ptr<char const* []> argv(new char const*[groups.size() + 3]);    // Includes room for final NULL element.
        argv[0] = scoperunner_path.c_str();
        argv[1] = config_file.c_str();
        int i = 2;
        for (auto it = group.begin(); it != group.end(); ++it)
        {
            argv[i++] = it->second.c_str();
        }
        argv[i] = NULL;

        // Fork/exec the scoperunner.
        pid_t pid;
        switch (pid = fork())
        {
            case -1:
            {
                throw SyscallException("cannot fork", errno);
            }
            case 0: // child
            {
                sigthread.reset_sigs(); // Need default signal mask in the new process
                execv(argv[0], const_cast<char* const*>(argv.get()));
                throw SyscallException("cannot exec " + scoperunner_path, errno);
            }
        }

        // Parent
        sigthread.add_child(pid, argv.get());
    }
}

} // namespace

int
main(int argc, char* argv[])
{
    prog_name = basename(argv[0]);
    if (argc != 2)
    {
        cerr << "usage: " << prog_name << " configfile" << endl;
        return 2;
    }
    char const* const config_file = argv[1];

    int exit_status = 0;

    // Run a separate thread to deal with SIGCHLD. This allows us to report when a scope process exits abnormally.
    SignalThread signal_thread;

    try
    {
        RuntimeImpl::UPtr runtime = RuntimeImpl::create("Registry", config_file);

        string identity = runtime->registry_identity();

        // Collect the registry config data.

        string mw_kind;
        string mw_endpoint;
        string mw_configfile;
        string scope_installdir;
        string scope_group_configdir;
        string oem_installdir;
        string oem_group_configdir;
        string scoperunner_path;
        {
            RegistryConfig c(identity, runtime->registry_configfile());
            mw_kind = c.mw_kind();
            mw_endpoint = c.endpoint();
            mw_configfile = c.mw_configfile();
            scope_installdir = c.scope_installdir();
            scope_group_configdir = c.scope_group_configdir();
            oem_installdir = c.oem_installdir();
            oem_group_configdir = c.oem_group_configdir();
            scoperunner_path = c.scoperunner_path();
        } // Release memory for config parser

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
        map<string, string> all_scopes;
        all_scopes.insert(overrideable_scopes.begin(), overrideable_scopes.end());
        all_scopes.insert(fixed_scopes.begin(), fixed_scopes.end());

        // Clear memory for original maps.
        map<string, string>().swap(fixed_scopes);
        map<string, string>().swap(overrideable_scopes);

        // Create the set of scope groups for Canonical and OEM scopes.
        auto canonical_groups = create_scope_groups(scope_group_configdir, all_scopes);
        auto oem_groups = create_scope_groups(oem_group_configdir, all_scopes);

        MiddlewareBase::SPtr middleware = runtime->factory()->create(identity, mw_kind, mw_configfile);

        // Add the registry implementation to the middleware.
        RegistryObject::SPtr registry(new RegistryObject);
        middleware->add_registry_object(runtime->registry_identity(), registry);

        // Add a proxy for each scope to the lookup table.
        // We do this before starting any of the scopes, so aggregating scopes don't get a lookup failure if
        // they look for another scope in the registry.
        for (auto pair : all_scopes)
        {
            registry->add(pair.first, middleware->create_scope_proxy(pair.first));
        }

        // Start a scoperunner for each Canonical scope group and add the corresponding proxies to the registry
        run_scopes(signal_thread, scoperunner_path, config_file, canonical_groups);

        // Start a scoperunner for each OEM scope group and add the corresponding proxies to the registry
        // TODO: run_scopes(signal_thread, scoperunner_path, config_file, oem_groups);

        // Wait until we are done.
        middleware->wait_for_shutdown();
    }
    catch (unity::Exception const& e)
    {
        error(e.to_string());
        exit_status = 1;
    }
    catch (std::exception const& e)
    {
        error(e.what());
        exit_status = 1;
    }
    catch (string const& e)
    {
        error("fatal error: " + e);
        exit_status = 1;
    }
    catch (char const* e)
    {
        error(string("fatal error: ") + e);
        exit_status = 1;
    }
    catch (...)
    {
        error("terminated due to unknown exception");
        exit_status = 1;
    }

    return exit_status;
}
