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
#include "ScopesWatcher.h"

#include <unity/scopes/internal/IniSettingsSchema.h>
#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/scopes/internal/RuntimeConfig.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeConfig.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <wordexp.h>

using namespace scoperegistry;
using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::util;
using namespace boost;

char const* prog_name;

namespace
{

void error(string const& msg)
{
    assert(!msg.empty());
    cerr << prog_name << ": " << msg << endl;
}

// if path is an absolute path, just return it. otherwise, append it to scopedir.
filesystem::path relative_scope_path_to_abs_path(filesystem::path const& path, filesystem::path const& scopedir)
{
    if (path.is_relative())
    {
        return scopedir / path;
    }
    return path;
}

// throwing an exception without joining with a thread is a bad, bad thing to do, so let's RAII to avoid doing it
struct SignalThreadWrapper
{
    std::shared_ptr<::core::posix::SignalTrap> termination_trap;
    std::shared_ptr<::core::posix::SignalTrap> child_trap;
    std::unique_ptr<::core::posix::ChildProcess::DeathObserver> death_observer;
    std::thread termination_trap_worker;
    std::thread child_trap_worker;

    SignalThreadWrapper() :
        // We shutdown the runtime whenever these signals happen.
        termination_trap(::core::posix::trap_signals_for_all_subsequent_threads(
            {
                ::core::posix::Signal::sig_int,
                ::core::posix::Signal::sig_hup,
                ::core::posix::Signal::sig_term
            })),
        // And we maintain our list of processes with the help of sig_chld.
        child_trap(::core::posix::trap_signals_for_all_subsequent_threads(
            {
                ::core::posix::Signal::sig_chld
            })),
        // The death observer is required to make sure that we reap all child processes
        // whenever multiple sigchld's are compressed together.
        death_observer(::core::posix::ChildProcess::DeathObserver::create_once_with_signal_trap(child_trap)),
        // Starting up both traps.
        termination_trap_worker([&]() { termination_trap->run(); }),
        child_trap_worker([&]() { child_trap->run(); })
    {
    }

    ::core::Signal<::core::posix::Signal>& signal_raised()
    {
        return termination_trap->signal_raised();
    }

    ~SignalThreadWrapper()
    {
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
    }
};

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

    auto config_files = find_install_dir_configs(scope_installdir, ".ini", error);
    for (auto&& pair : config_files)
    {
        if (boost::ends_with(pair.second, "-settings.ini"))
        {
            // Don't try to parse settings metadata file as scope config file.
            continue;
        }
        try
        {
            ScopeConfig config(pair.second);
            if (config.overrideable())
            {
                overrideable_scopes[pair.first] = pair.second;
            }
            else
            {
                fixed_scopes[pair.first] = pair.second;
            }
        }
        catch (unity::Exception const& e)
        {
            error("ignoring scope \"" + pair.first + "\": configuration error:\n" + e.what());
        }
    }

    if (!oem_installdir.empty())
    {
        try
        {
            auto oem_paths = find_install_dir_configs(oem_installdir, ".ini", error);
            for (auto&& path : oem_paths)
            {
                if (fixed_scopes.find(path.first) == fixed_scopes.end())
                {
                    overrideable_scopes[path.first] = path.second;  // Replaces scope if it was present already
                }
                else
                {
                    error("ignoring non-overrideable scope config \"" + path.second + "\" in OEM directory " + oem_installdir);
                }
            }
        }
        catch (FileException const& e)
        {
            error(e.what());
            error("could not open OEM installation directory, ignoring OEM scopes");
        }
    }

    // Combine fixed_scopes and overrideable_scopes now.
    fixed_scopes.insert(overrideable_scopes.begin(), overrideable_scopes.end());
    return fixed_scopes;
}

map<string, string> find_click_scopes(map<string, string> const& local_scopes, string const& click_installdir)
{
    map<string, string> click_scopes;

    if (!click_installdir.empty())
    {
        try
        {
            auto click_paths = find_install_dir_configs(click_installdir, ".ini", error);
            for (auto&& pair : click_paths)
            {
                if (boost::ends_with(pair.second, "-settings.ini"))
                {
                    // Don't try to parse settings metadata file as scope config file.
                    continue;
                }
                if (local_scopes.find(pair.first) == local_scopes.end())
                {
                    click_scopes[pair.first] = pair.second;
                }
                else
                {
                    error("ignoring non-overrideable scope config \"" + pair.second + "\" in click directory " + click_installdir);
                }
            }
        }
        catch (FileException const& e)
        {
            error(e.what());
            error("could not open Click installation directory, ignoring Click scopes");
        }
    }

    return click_scopes;
}

static bool starts_with(const string& compare, const string& prefix)
{
    bool result = false;
    if (compare.length() >= prefix.length())
    {
        result = (compare.compare(0, prefix.length(), prefix) == 0);
    }
    return result;
}

static bool is_uri(const string& path)
{
    return starts_with(path, "color://")
            || starts_with(path, "file://")
            || starts_with(path, "ftp://")
            || starts_with(path, "gradient://")
            || starts_with(path, "http://")
            || starts_with(path, "https://")
            || starts_with(path, "image://");
}

void convert_relative_attribute(VariantMap& inner_map, const string& id, const filesystem::path& scope_dir)
{
    auto logo_it = inner_map.find(id);
    if (logo_it != inner_map.end()
            && logo_it->second.which() == Variant::String)
    {
        auto path = logo_it->second.get_string();

        // Do not normalize supported URI schemas
        if(!is_uri(path))
        {
            logo_it->second = Variant(
                    relative_scope_path_to_abs_path(path, scope_dir).native());
        }
    }
}

// For each scope, open the config file for the scope, create the metadata info from the config,
// and add an entry to the RegistryObject.
// If the scope uses settings, also parse the settings file and add the settings to the metadata.

void add_local_scope(RegistryObject::SPtr const& registry,
                     pair<string, string> const& scope,
                     MiddlewareBase::SPtr const& mw,
                     string const& scoperunner_path,
                     string const& config_file,
                     bool click,
                     int timeout_ms)
{
    unique_ptr<ScopeMetadataImpl> mi(new ScopeMetadataImpl(mw.get()));
    string scope_config(scope.second);
    ScopeConfig sc(scope_config);

    filesystem::path scope_path(scope_config);
    filesystem::path scope_dir(scope_path.parent_path());
    filesystem::path settings_schema_path(scope_dir / (scope.first + "-settings.ini"));

    mi->set_settings_definitions(VariantArray());
    try
    {
        IniSettingsSchema::UPtr schema;

        // File is optional, so don't generate noise if it isn't there.
        if (filesystem::exists(settings_schema_path))
        {
            schema = IniSettingsSchema::create(settings_schema_path.native());
        }
        else if (sc.location_data_needed())
        {
            // TODO: HACK: See bug #1393438 and the comments in IniSettingsSchema.cpp and
            //             JsonSettingsSchema.cpp.
            // We always need to create a settings schema if the scope wants location data
            schema = IniSettingsSchema::create_empty();
        }

        if (schema)
        {
            if (sc.location_data_needed())
            {
                schema->add_location_setting();
            }
            mi->set_settings_definitions(schema->definitions());
        }
    }
    catch (std::exception const& e)
    {
        error("ignoring settings schema file " + settings_schema_path.native()
              + " for scope " + scope.first + ": " + e.what());
    }

    mi->set_scope_id(scope.first);
    mi->set_display_name(sc.display_name());
    mi->set_description(sc.description());
    mi->set_author(sc.author());
    mi->set_invisible(sc.invisible());
    mi->set_appearance_attributes(sc.appearance_attributes());
    mi->set_child_scope_ids(sc.child_scope_ids());
    mi->set_version(sc.version());
    mi->set_keywords(sc.keywords());
    mi->set_is_aggregator(sc.is_aggregator());

    // Prepend scope_dir to pageheader logo path if logo path is relative.
    // TODO: Once we have type-safe parsing in the config files, remove
    //       the calls to which(). We should be able to rely on things
    //       having the correct type. That's the whole point of having
    //       the *Config classes.
    auto app_attrs = sc.appearance_attributes();
    auto ph_it = app_attrs.find("page-header");
    if (ph_it != app_attrs.end() && ph_it->second.which() == Variant::Dict)
    {
        auto inner_map = ph_it->second.get_dict();
        convert_relative_attribute(inner_map, "logo", scope_dir);
        convert_relative_attribute(inner_map, "background", scope_dir);
        convert_relative_attribute(inner_map, "navigation-background", scope_dir);
        app_attrs["page-header"] = Variant(inner_map);
    }
    mi->set_appearance_attributes(app_attrs);

    mi->set_scope_directory(scope_dir.native());
    mi->set_results_ttl_type(sc.results_ttl_type());
    mi->set_location_data_needed(sc.location_data_needed());

    try
    {
        auto art = sc.art();
        if (!is_uri(art))
        {
            art = relative_scope_path_to_abs_path(art, scope_dir).native();
        }
        mi->set_art(art);
    }
    catch (NotFoundException const&)
    {
    }
    try
    {
        auto icon = sc.icon();
        if (!is_uri(icon))
        {
            icon = relative_scope_path_to_abs_path(icon, scope_dir).native();
        }
        mi->set_icon(icon);
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

    ScopeProxy proxy = ScopeImpl::create(mw->create_scope_proxy(scope.first), scope.first);
    mi->set_proxy(proxy);
    auto meta = ScopeMetadataImpl::create(std::move(mi));

    RegistryObject::ScopeExecData exec_data;
    exec_data.scope_id = scope.first;
    // get custom scope runner executable, if not set use default scoperunner
    exec_data.scoperunner_path = scoperunner_path;

    if (click)
    {
        exec_data.confinement_profile =
                scope_dir.filename().native();
    }

    // Check if this scope has requested debug mode, if so, disable process timeout
    if (sc.debug_mode())
    {
        exec_data.timeout_ms = -1;
    }
    else
    {
        exec_data.timeout_ms = timeout_ms;
    }

    try
    {
        exec_data.custom_exec = convert_exec_rel_to_abs(scope.first, scope_dir, sc.scope_runner());
    }
    catch (NotFoundException const&)
    {
    }
    exec_data.runtime_config = config_file;
    exec_data.scope_config = scope.second;
    exec_data.debug_mode = sc.debug_mode();

    registry->add_local_scope(scope.first, std::move(meta), exec_data);
}

void add_local_scopes(RegistryObject::SPtr const& registry,
                      map<string, string> const& all_scopes,
                      MiddlewareBase::SPtr const& mw,
                      string const& scoperunner_path,
                      string const& config_file,
                      bool click,
                      int timeout_ms)
{
    for (auto&& pair : all_scopes)
    {
        try
        {
            add_local_scope(registry, pair, mw, scoperunner_path, config_file, click, timeout_ms);
        }
        catch (unity::Exception const& e)
        {
            error("ignoring scope \"" + pair.first + "\": cannot create metadata: " + e.what());
        }
    }
}

} // namespace

// Usage: scoperegistry [runtime.ini] [scope.ini]...
//
// If no runtime config file is specified, the default location (/usr/lib/<arch>/unity-scopes/Runtime.ini)
// is assumed.
// If additional scope configuration files are specified, the corresponding scopes will be added
// to the registry (overriding any scopes that are found via config files reached via Runtime.ini).

void usage(ostream& s = cerr)
{
    s << "usage: " << prog_name << " [runtime.ini] [scope.ini]..." << endl;
}

int main(int argc, char* argv[])
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
        SignalThreadWrapper signal_handler_wrapper;

        // And finally creating our runtime.
        string identity;
        string ss_reg_id;
        RuntimeImpl::SPtr runtime;
        {
            RuntimeConfig rt_config(config_file);
            runtime = RuntimeImpl::create(rt_config.registry_identity(), config_file);

            identity = runtime->registry_identity();
            ss_reg_id = runtime->ss_registry_identity();

            // Make sure that the cache and app directories exist.
            string cache_root = rt_config.cache_directory() + "/leaf-net";
            make_directories(cache_root, 0700);

            string app_root = rt_config.app_directory();
            make_directories(app_root, 0700);
        } // Release memory for config parser

        // Make sure that the confined scope tmp directory exists.
        string tmp_dir = string("/run/user/") + std::to_string(geteuid()) + "/scopes/leaf-net";
        make_directories(tmp_dir, 0700);

        // Make sure that the directory for .desktop files exists.
        string desktop_files_dir = RegistryObject::desktop_files_dir();
        if (!desktop_files_dir.empty())
        {
            make_directories(desktop_files_dir, 0700);
        }

        // Collect the registry config data.

        string mw_kind;
        string scope_installdir;
        string oem_installdir;
        string click_installdir;
        string scoperunner_path;
        int process_timeout;
        {
            RegistryConfig c(identity, runtime->registry_configfile());
            mw_kind = c.mw_kind();
            scope_installdir = c.scope_installdir();
            oem_installdir = c.oem_installdir();
            click_installdir = c.click_installdir();
            scoperunner_path = c.scoperunner_path();
            process_timeout = c.process_timeout();
        } // Release memory for config parser

        // Inform the signal thread that it should shutdown the runtime
        // if we get a termination signal.
        signal_handler_wrapper.signal_raised().connect([runtime](::core::posix::Signal signal)
        {
            switch(signal)
            {
            case ::core::posix::Signal::sig_int:
            case ::core::posix::Signal::sig_hup:
            case ::core::posix::Signal::sig_term:
                runtime->destroy();
                break;
            default:
                break;
            }
        });

        // The registry object stores the local and remote scopes
        MiddlewareBase::SPtr middleware = runtime->factory()->find(identity, mw_kind);
        Executor::SPtr executor = std::make_shared<Executor>();
        RegistryObject::SPtr registry(new RegistryObject(*signal_handler_wrapper.death_observer, executor, middleware, true));

        // Add the metadata for each scope to the lookup table.
        // We do this before starting any of the scopes, so aggregating scopes don't get a lookup failure if
        // they look for another scope in the registry.

        auto local_scopes = find_local_scopes(scope_installdir, oem_installdir);
        auto click_scopes = find_click_scopes(local_scopes, click_installdir);

        // Before we add the local scopes, we check whether any scopes were explicitly specified
        // on the command line. If so, scopes on the command line override scopes in
        // configuration files.
        for (auto i = 2; i < argc; ++i)
        {
            filesystem::path path(argv[i]);
            string scope_id = path.stem().native();
            local_scopes[scope_id] = argv[i];                   // operator[] overwrites pre-existing entries
        }

        add_local_scopes(registry, local_scopes, middleware, scoperunner_path, config_file, false, process_timeout);
        add_local_scopes(registry, click_scopes, middleware, scoperunner_path, config_file, true, process_timeout);
        if (ss_reg_id.empty())
        {
            error("no remote registry configured, only local scopes will be available");
        }
        else
        {
            registry->set_remote_registry(middleware->ss_registry_proxy());
        }

        // Configure watches for scope install directories
        auto local_watch_lambda = [registry, &middleware, &scoperunner_path, &config_file, process_timeout]
                                  (pair<string, string> const& scope)
        {
            try
            {
                add_local_scope(registry, scope, middleware, scoperunner_path, config_file, false, process_timeout);
            }
            catch (unity::Exception const& e)
            {
                error("ignoring installed scope \"" + scope.first + "\": cannot create metadata: " + e.what());
            }
        };
        ScopesWatcher local_scopes_watcher(registry, local_watch_lambda, runtime->logger());
        local_scopes_watcher.add_install_dir(scope_installdir);
        local_scopes_watcher.add_install_dir(oem_installdir);

        auto click_watch_lambda = [registry, &middleware, &scoperunner_path, &config_file, process_timeout]
                                  (pair<string, string> const& scope)
        {
            try
            {
                add_local_scope(registry, scope, middleware, scoperunner_path, config_file, true, process_timeout);
            }
            catch (unity::Exception const& e)
            {
                error("ignoring installed scope \"" + scope.first + "\": cannot create metadata: " + e.what());
            }
        };
        ScopesWatcher click_scopes_watcher(registry, click_watch_lambda, runtime->logger());
        click_scopes_watcher.add_install_dir(click_installdir);

        // Let's add the registry's state receiver to the middleware so that scopes can inform
        // the registry of state changes.
        middleware->add_state_receiver_object("StateReceiver", registry->state_receiver());

        // Now that the registry table is populated, we can add the registry to the middleware, so
        // it starts processing incoming requests.
        auto p = middleware->add_registry_object(runtime->registry_identity(), registry);

        // Drop our shared_ptr to the RegistryObject. This means that the registry object
        // is kept alive only via the shared_ptr held by the middleware. If the middleware
        // shuts down, it clears out the active servant map, which destroys the registry
        // object. The registry object kills all its child processes as part of its clean-up.
        registry = nullptr;

        // Wait until we are done, which happens if we receive a termination signal.
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
