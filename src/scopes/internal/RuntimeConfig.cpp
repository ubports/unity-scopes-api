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
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/ScopeExceptions.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/constants.hpp>
#include <boost/algorithm/string/split.hpp>
#include <unity/UnityExceptions.h>

#include <iostream>  // TODO: remove this

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
const string log_dir_key = "LogDir";
const string max_log_file_size_key = "Log.MaxFileSize";
const string max_log_dir_size_key = "Log.MaxDirSize";
const string trace_channels_key = "Log.TraceChannels";

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
        max_log_file_size_ = DFLT_MAX_LOG_FILE_SIZE;
        max_log_dir_size_ = DFLT_MAX_LOG_DIR_SIZE;
        cache_directory_ = default_cache_directory();
        app_directory_ = default_app_directory();
        config_directory_ = default_config_directory();
        log_directory_ = default_log_directory();
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
            catch (ResourceException const&)
            {
                throw_ex("No " + cache_dir_key + " configured and failed to get default");
            }
        }

        app_directory_ = get_optional_string(runtime_config_group, app_dir_key);
        if (app_directory_.empty())
        {
            try
            {
                app_directory_ = default_app_directory();
            }
            catch (ResourceException const&)
            {
                throw_ex("No " + app_dir_key + " configured and failed to get default");
            }
        }

        // UNITY_SCOPES_CONFIG_DIR env var can be used to override the value of ConfigDir key of Runtime.ini
        char const* scopes_cfg_dir_override = getenv("UNITY_SCOPES_CONFIG_DIR");
        config_directory_ = (scopes_cfg_dir_override != nullptr && *scopes_cfg_dir_override != '\0') ?
            string(scopes_cfg_dir_override) : get_optional_string(runtime_config_group, config_dir_key);
        if (config_directory_.empty())
        {
            try
            {
                config_directory_ = default_config_directory();
            }
            catch (ResourceException const&)
            {
                throw_ex("No " + config_dir_key + " configured and failed to get default");
            }
        }

#if 0
        try
        {
            // If explicitly set to the empty string, we succeed here.
            log_directory_ = parser()->get_string(runtime_config_group, log_dir_key);
        }
        catch (LogicException const&)
        {
            // Use default value.
            try
            {
                log_directory_ = default_log_directory();
            }
            catch (ResourceException const&)
            {
                throw_ex("No " + log_dir_key + " configured and failed to get default");
            }
        }
#else
        log_directory_ = get_optional_string(runtime_config_group, log_dir_key);
        cerr << "log_directory_: " << log_directory_ << endl;
#endif

        // Check if we have an override for the log directory.
        char const* logdir = getenv("UNITY_SCOPES_LOGDIR");
        if (logdir)
        {
            log_directory_ = logdir;
        }

        max_log_file_size_ = get_optional_int(runtime_config_group, max_log_file_size_key, DFLT_MAX_LOG_FILE_SIZE);
        if (max_log_file_size_ < 1024)
        {
            throw_ex("Illegal value (" + to_string(max_log_file_size_) + ") for " + max_log_file_size_key
                     + ": value must be > 1024");
        }

        max_log_dir_size_ = get_optional_int(runtime_config_group, max_log_dir_size_key, DFLT_MAX_LOG_DIR_SIZE);
        if (max_log_dir_size_ <= max_log_file_size_)
        {
            throw_ex("Illegal value (" + to_string(max_log_dir_size_) + ") for " + max_log_dir_size_key
                     + ": value must be > " + max_log_file_size_key + " (" + to_string(max_log_file_size_) + ")");
        }

        try
        {
            trace_channels_ = parser()->get_string_array(runtime_config_group, trace_channels_key);
        }
        catch (LogicException const&)
        {
            // No TraceChannels configured.
        }

        // Check if we have an override for the trace channels.
        char const* tc = getenv("UNITY_SCOPES_LOG_TRACECHANNELS");
        if (tc && *tc != '\0')
        {
            vector<string> channels;
            split(channels, tc, boost::is_any_of(";"), boost::token_compress_on);
            trace_channels_ = channels;
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
                                                config_dir_key,
                                                log_dir_key,
                                                max_log_file_size_key,
                                                max_log_dir_size_key,
                                                trace_channels_key
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

string RuntimeConfig::log_directory() const
{
    return log_directory_;
}

int RuntimeConfig::max_log_file_size() const
{
    return max_log_file_size_;
}

int RuntimeConfig::max_log_dir_size() const
{
    return max_log_dir_size_;
}

vector<string> RuntimeConfig::trace_channels() const
{
    return trace_channels_;
}

string RuntimeConfig::default_cache_directory()
{
    char const* home = getenv("HOME");
    if (!home || *home == '\0')
    {
        throw ResourceException("RuntimeConfig::default_cache_directory(): $HOME not set");
    }
    return string(home) + "/" + DFLT_HOME_CACHE_SUBDIR;
}

string RuntimeConfig::default_app_directory()
{
    char const* home = getenv("HOME");
    if (!home || *home == '\0')
    {
        throw ResourceException("RuntimeConfig::default_app_directory(): $HOME not set");
    }
    return string(home) + "/" + DFLT_HOME_APP_SUBDIR;
}

string RuntimeConfig::default_config_directory()
{
    char const* home = getenv("HOME");
    if (!home || *home == '\0')
    {
        throw ResourceException("RuntimeConfig::default_config_directory(): $HOME not set");
    }
    return string(home) + "/" + DFLT_HOME_CONFIG_SUBDIR;
}

string RuntimeConfig::default_log_directory()
{
    char const* home = getenv("HOME");
    if (!home || *home == '\0')
    {
        throw ResourceException("RuntimeConfig::default_log_directory(): $HOME not set");
    }
    string dir = string(home) + "/" + DFLT_HOME_LOG_SUBDIR;
    make_directories(dir, 0700);

    return dir;
}

} // namespace internal

} // namespace scopes

} // namespace unity
