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

#include <unity/scopes/internal/RuntimeConfig.h>

#include <unity/scopes/internal/DfltConfig.h>
#include <unity/UnityExceptions.h>

#include <stdlib.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace
{

const string runtime_config_group = "Runtime";
const string registry_identity_key = "Registry.Identity";
const string registry_configfile_key = "Registry.ConfigFile";
const string ss_registry_identity_key = "Smartscopes.Registry.Identity";
const string ss_configfile_key = "Smartscopes.ConfigFile";
const string default_middleware_key = "Default.Middleware";
const string default_middleware_configfile_key = ".ConfigFile";
const string reap_expiry_key = "Reap.Expiry";
const string reap_interval_key = "Reap.Interval";
const string cache_dir_key = "CacheDir";
const string app_dir_key = "AppDir";
const string config_dir_key = "ConfigDir";

}  // namespace

RuntimeConfig::RuntimeConfig(string const& configfile) :
    ConfigBase(configfile)
{
    if (configfile.empty())  // Default config
    {
        registry_identity_ = DFLT_REGISTRY_ID;
        registry_configfile_ = DFLT_REGISTRY_INI;
        ss_registry_identity_ = DFLT_SS_REGISTRY_ID;
        ss_configfile_ = DFLT_SS_REGISTRY_INI;
        default_middleware_ = DFLT_MIDDLEWARE;
        default_middleware_configfile_ = DFLT_ZMQ_MIDDLEWARE_INI;
        reap_expiry_ = DFLT_REAP_EXPIRY;
        reap_interval_ = DFLT_REAP_INTERVAL;
        try
        {
            cache_directory_ = default_cache_directory();
            app_directory_ = default_app_directory();
            config_directory_ = default_config_directory();
        }
        catch (ResourceException const& e)
        {
            throw_ex("Failed to get default data directory");
        }
    }
    else
    {
        registry_identity_ = get_optional_string(runtime_config_group, registry_identity_key);
        registry_configfile_ = get_optional_string(runtime_config_group, registry_configfile_key);
        ss_configfile_ = get_optional_string(runtime_config_group, ss_configfile_key);
        ss_registry_identity_ = get_optional_string(runtime_config_group, ss_registry_identity_key, DFLT_SS_REGISTRY_ID);
        default_middleware_ = get_middleware(runtime_config_group, default_middleware_key);
        default_middleware_configfile_ = get_optional_string(runtime_config_group,
                                                             default_middleware_ + default_middleware_configfile_key,
                                                             DFLT_MIDDLEWARE_INI);
        reap_expiry_ = get_optional_int(runtime_config_group, reap_expiry_key, DFLT_REAP_EXPIRY);
        if (reap_expiry_ < 1 && reap_expiry_ != -1)
        {
            throw_ex("Illegal value (" + to_string(reap_expiry_) + ") for " + reap_expiry_key + ": value must be > 0");
        }
        reap_interval_ = get_optional_int(runtime_config_group, reap_interval_key, DFLT_REAP_INTERVAL);
        if (reap_interval_ < 1 && reap_interval_ != -1)
        {
            throw_ex("Illegal value (" + to_string(reap_interval_) + ") for " + reap_interval_key + ": value must be > 0");
        }
        cache_directory_ = get_optional_string(runtime_config_group, cache_dir_key);
        if (cache_directory_.empty())
        {
            try
            {
                cache_directory_ = default_cache_directory();
            }
            catch (ResourceException const& e)
            {
                throw_ex("No CacheDir configured and failed to get default");
            }
        }
        app_directory_ = get_optional_string(runtime_config_group, app_dir_key);
        if (app_directory_.empty())
        {
            try
            {
                app_directory_ = default_app_directory();
            }
            catch (ResourceException const& e)
            {
                throw_ex("No AppDir configured and failed to get default");
            }
        }
        config_directory_ = get_optional_string(runtime_config_group, config_dir_key);
        if (config_directory_.empty())
        {
            try
            {
                config_directory_ = default_config_directory();
            }
            catch (ResourceException const& e)
            {
                throw_ex("No ConfigDir configured and failed to get default");
            }
        }
    }

    KnownEntries const known_entries = {
                                          {  runtime_config_group,
                                             {
                                                registry_identity_key,
                                                registry_configfile_key,
                                                ss_registry_identity_key,
                                                ss_configfile_key,
                                                default_middleware_key,
                                                default_middleware_ + default_middleware_configfile_key,
                                                reap_expiry_key,
                                                reap_interval_key,
                                                cache_dir_key,
                                                app_dir_key,
                                                config_dir_key
                                             }
                                          }
                                       };
    check_unknown_entries(known_entries);
}

RuntimeConfig::~RuntimeConfig()
{
}

string RuntimeConfig::registry_identity() const
{
    return registry_identity_;
}

string RuntimeConfig::registry_configfile() const
{
    return registry_configfile_;
}

string RuntimeConfig::ss_registry_identity() const
{
    return ss_registry_identity_;
}

string RuntimeConfig::ss_configfile() const
{
    return ss_configfile_;
}

string RuntimeConfig::default_middleware() const
{
    return default_middleware_;
}

string RuntimeConfig::default_middleware_configfile() const
{
    return default_middleware_configfile_;
}

int RuntimeConfig::reap_expiry() const
{
    return reap_expiry_;
}

int RuntimeConfig::reap_interval() const
{
    return reap_interval_;
}

string RuntimeConfig::cache_directory() const
{
    return cache_directory_;
}

string RuntimeConfig::app_directory() const
{
    return app_directory_;
}

string RuntimeConfig::config_directory() const
{
    return config_directory_;
}

string RuntimeConfig::default_cache_directory()
{
    char const* home = getenv("HOME");
    if (!home || *home == '\0')
    {
        throw ResourceException("RuntimeConfig::default_cache_directory(): $HOME not set");
    }
    return string(home) + "/.local/share/unity-scopes";
}

string RuntimeConfig::default_app_directory()
{
    char const* home = getenv("HOME");
    if (!home || *home == '\0')
    {
        throw ResourceException("RuntimeConfig::default_app_directory(): $HOME not set");
    }
    return string(home) + "/.local/share";
}

string RuntimeConfig::default_config_directory()
{
    char const* home = getenv("HOME");
    if (!home || *home == '\0')
    {
        throw ResourceException("RuntimeConfig::default_config_directory(): $HOME not set");
    }
    return string(home) + "/.config/unity-scopes";
}

} // namespace internal

} // namespace scopes

} // namespace unity
