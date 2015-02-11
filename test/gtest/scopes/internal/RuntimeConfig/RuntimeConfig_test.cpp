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
#include <unity/scopes/ScopeExceptions.h>

#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.

#include <gtest/gtest.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RuntimeConfig, basic)
{
    setenv("HOME", TEST_SRC_DIR, 1);

    RuntimeConfig c("");
    EXPECT_EQ("Registry", c.registry_identity());
    EXPECT_EQ(DFLT_REGISTRY_INI, c.registry_configfile());
    EXPECT_EQ("SSRegistry", c.ss_registry_identity());
    EXPECT_EQ(DFLT_SS_REGISTRY_INI, c.ss_configfile());
    EXPECT_EQ("Zmq", c.default_middleware());
    EXPECT_EQ(DFLT_MIDDLEWARE_INI, c.default_middleware_configfile());
    EXPECT_EQ(DFLT_REAP_EXPIRY, c.reap_expiry());
    EXPECT_EQ(DFLT_REAP_INTERVAL, c.reap_interval());
    EXPECT_EQ(DFLT_MAX_LOG_FILE_SIZE, c.max_log_file_size());
    EXPECT_EQ(DFLT_MAX_LOG_DIR_SIZE, c.max_log_dir_size());
    EXPECT_TRUE(c.trace_channels().empty());
}

TEST(RuntimeConfig, complete)
{
    RuntimeConfig c(TEST_SRC_DIR "/Complete.ini");
    EXPECT_EQ("R.Id", c.registry_identity());
    EXPECT_EQ("R.Config", c.registry_configfile());
    EXPECT_EQ("SS.R.Id", c.ss_registry_identity());
    EXPECT_EQ("SS.Config", c.ss_configfile());
    EXPECT_EQ("Zmq", c.default_middleware());
    EXPECT_EQ("Z.Config", c.default_middleware_configfile());
    EXPECT_EQ(500, c.reap_expiry());
    EXPECT_EQ(100, c.reap_interval());
    EXPECT_EQ("CacheD", c.cache_directory());
    EXPECT_EQ("AppD", c.app_directory());
    EXPECT_EQ("ConfigD", c.config_directory());
    EXPECT_EQ("LogD", c.log_directory());
    EXPECT_EQ(10000, c.max_log_file_size());
    EXPECT_EQ(20000, c.max_log_dir_size());
    EXPECT_EQ(vector<string>{ "IPC" }, c.trace_channels());
}

TEST(RuntimeConfig, _default_cache_dir)
{
    setenv("HOME", TEST_SRC_DIR, 1);

    RuntimeConfig c("");
    EXPECT_EQ(TEST_SRC_DIR "/.local/share/unity-scopes", c.cache_directory());
}

TEST(RuntimeConfig, overridden_cache_dir)
{
    unsetenv("HOME");

    RuntimeConfig c(TEST_SRC_DIR "/CacheDir.ini");
    EXPECT_EQ("cachedir", c.cache_directory());
}

TEST(RuntimeConfig, overridden_cache_dir_with_home_dir)
{
    setenv("HOME", TEST_SRC_DIR, 1);

    RuntimeConfig c(TEST_SRC_DIR "/CacheDir.ini");
    EXPECT_EQ("cachedir", c.cache_directory());
}

TEST(RuntimeConfig, overridden_app_dir)
{
    unsetenv("HOME");

    RuntimeConfig c(TEST_SRC_DIR "/CacheDir.ini");
    EXPECT_EQ("appdir", c.app_directory());
}

TEST(RuntimeConfig, overridden_app_dir_with_home_dir)
{
    setenv("HOME", TEST_SRC_DIR, 1);

    RuntimeConfig c(TEST_SRC_DIR "/ConfigDir.ini");
    EXPECT_EQ("appdir", c.app_directory());
}

TEST(RuntimeConfig, overridden_config_dir)
{
    unsetenv("HOME");

    RuntimeConfig c(TEST_SRC_DIR "/ConfigDir.ini");
    EXPECT_EQ("configdir", c.config_directory());
}

TEST(RuntimeConfig, overridden_config_dir_with_home_dir)
{
    setenv("HOME", TEST_SRC_DIR, 1);

    RuntimeConfig c(TEST_SRC_DIR "/ConfigDir.ini");
    EXPECT_EQ("configdir", c.config_directory());
}

TEST(RuntimeConfig, overridden_log_dir)
{
    unsetenv("HOME");

    RuntimeConfig c(TEST_SRC_DIR "/LogDir.ini");
    EXPECT_EQ("logdir", c.log_directory());
}

TEST(RuntimeConfig, overridden_log_dir_with_home_dir)
{
    RuntimeConfig c(TEST_SRC_DIR "/LogDir.ini");
    EXPECT_EQ("logdir", c.log_directory());
}

TEST(RuntimeConfig, log_dir_env_var_override)
{
    setenv("UNITY_SCOPES_LOGDIR", "otherdir", 1);

    RuntimeConfig c(TEST_SRC_DIR "/LogDir.ini");
    EXPECT_EQ("otherdir", c.log_directory());
}

TEST(RuntimeConfig, trace_channels_env_var_override)
{
    setenv("UNITY_SCOPES_LOG_TRACECHANNELS", "ABC;XYZ;;DEF", 1);

    RuntimeConfig c(TEST_SRC_DIR "/Complete.ini");
    EXPECT_EQ((vector<string>{ "ABC", "XYZ", "DEF"}), c.trace_channels());
}

TEST(RuntimeConfig, exceptions)
{
    try
    {
        RuntimeConfig c(TEST_SRC_DIR "/BadMW.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/BadMW.ini\": Illegal value for Default.Middleware: "
                     "\"Foo\": legal values are \"Zmq\" and \"REST\"",
                     e.what());
    }

    try
    {
        RuntimeConfig c(TEST_SRC_DIR "/BadReapExpiry.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/BadReapExpiry.ini\": Illegal value (0) for "
                     "Reap.Expiry: value must be > 0",
                     e.what());
    }

    try
    {
        RuntimeConfig c(TEST_SRC_DIR "/BadReapInterval.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/BadReapInterval.ini\": Illegal value (0) for "
                     "Reap.Interval: value must be > 0",
                     e.what());
    }

    try
    {
        unsetenv("HOME");

        RuntimeConfig c(TEST_SRC_DIR "/NoCacheDir.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/NoCacheDir.ini\": No CacheDir configured and "
                     "failed to get default:\n    unity::ResourceException: RuntimeConfig::default_cache_directory(): "
                     "$HOME not set",
                     e.what());
    }

    try
    {
        unsetenv("HOME");

        RuntimeConfig c(TEST_SRC_DIR "/NoConfigDir.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/NoConfigDir.ini\": No ConfigDir configured and "
                     "failed to get default:\n    unity::ResourceException: RuntimeConfig::default_config_directory(): "
                     "$HOME not set",
                     e.what());
    }

    try
    {
        unsetenv("HOME");

        RuntimeConfig c(TEST_SRC_DIR "/NoLogDir.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        // Using regex here because error message returned by glib changed from Utopic to Vivid.
        // The final .* takes care of the difference. Note that, instead of using TEST_SRC_DIR, we
        // use .+. That's because, when building with bzr bd, we end up with a '+' in the path,
        // and that is a regex metacharacter, causing the match to fail.
        boost::regex r("unity::scopes::ConfigException: \".+/NoLogDir.ini\": "
                       "No LogDir configured and failed to get default:\\n"
                       "    unity::ResourceException: RuntimeConfig::default_log_directory\\(\\): \\$HOME not set:\\n"
                       "        unity::LogicException: Could not get string value \\(.+/NoLogDir.ini, .*");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        unsetenv("HOME");

        RuntimeConfig c(TEST_SRC_DIR "/NoAppDir.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ( "unity::scopes::ConfigException: \"" TEST_SRC_DIR "/NoAppDir.ini\": "
                      "No AppDir configured and failed to get default:\n"
                      "    unity::ResourceException: RuntimeConfig::default_app_directory(): $HOME not set",
                     e.what());
    }

    try
    {
        RuntimeConfig c(TEST_SRC_DIR "/BadLogFileSize.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/BadLogFileSize.ini\": "
                     "Illegal value (999) for Log.MaxFileSize: value must be > 1024",
                     e.what());
    }

    try
    {
        RuntimeConfig c(TEST_SRC_DIR "/BadLogDirSize.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/BadLogDirSize.ini\": "
                     "Illegal value (1024) for Log.MaxDirSize: value must be > Log.MaxFileSize (2048)",
                     e.what());
    }
}
