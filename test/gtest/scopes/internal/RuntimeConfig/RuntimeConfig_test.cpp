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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

class RuntimeConfigTest: public ::testing::Test {
    public:
        virtual void SetUp()
        {
            unsetenv("UNITY_SCOPES_CONFIG_DIR");
        }
};

TEST_F(RuntimeConfigTest, basic)
{
    setenv("HOME", TEST_DIR, 1);

    RuntimeConfig c("");
    EXPECT_EQ("Registry", c.registry_identity());
    EXPECT_EQ(DFLT_REGISTRY_INI, c.registry_configfile());
    EXPECT_EQ("SSRegistry", c.ss_registry_identity());
    EXPECT_EQ(DFLT_SS_REGISTRY_INI, c.ss_configfile());
    EXPECT_EQ("Zmq", c.default_middleware());
    EXPECT_EQ(DFLT_MIDDLEWARE_INI, c.default_middleware_configfile());
    EXPECT_EQ(DFLT_REAP_EXPIRY, c.reap_expiry());
    EXPECT_EQ(DFLT_REAP_INTERVAL, c.reap_interval());
    EXPECT_TRUE(c.trace_channels().empty());
}

TEST_F(RuntimeConfigTest, complete)
{
    RuntimeConfig c(TEST_DIR "/Complete.ini");
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
    EXPECT_EQ(vector<string>{ "IPC" }, c.trace_channels());
}

TEST_F(RuntimeConfigTest, _default_cache_dir)
{
    setenv("HOME", TEST_DIR, 1);

    RuntimeConfig c("");
    EXPECT_EQ(TEST_DIR "/.local/share/unity-scopes", c.cache_directory());
}

TEST_F(RuntimeConfigTest, overridden_cache_dir)
{
    unsetenv("HOME");

    RuntimeConfig c(TEST_DIR "/CacheDir.ini");
    EXPECT_EQ("cachedir", c.cache_directory());
}

TEST_F(RuntimeConfigTest, overridden_cache_dir_with_home_dir)
{
    setenv("HOME", TEST_DIR, 1);

    RuntimeConfig c(TEST_DIR "/CacheDir.ini");
    EXPECT_EQ("cachedir", c.cache_directory());
}

TEST_F(RuntimeConfigTest, overridden_app_dir)
{
    unsetenv("HOME");

    RuntimeConfig c(TEST_DIR "/CacheDir.ini");
    EXPECT_EQ("appdir", c.app_directory());
}

TEST_F(RuntimeConfigTest, overridden_app_dir_with_home_dir)
{
    setenv("HOME", TEST_DIR, 1);

    RuntimeConfig c(TEST_DIR "/ConfigDir.ini");
    EXPECT_EQ("appdir", c.app_directory());
}

TEST_F(RuntimeConfigTest, overridden_config_dir)
{
    unsetenv("HOME");

    RuntimeConfig c(TEST_DIR "/ConfigDir.ini");
    EXPECT_EQ("configdir", c.config_directory());
}

TEST_F(RuntimeConfigTest, overridden_config_dir_with_home_dir)
{
    setenv("HOME", TEST_DIR, 1);

    RuntimeConfig c(TEST_DIR "/ConfigDir.ini");
    EXPECT_EQ("configdir", c.config_directory());
}

TEST_F(RuntimeConfigTest, overridden_config_dir_with_unity_scopes_cfg_dir)
{
    setenv("HOME", TEST_DIR, 1);
    setenv("UNITY_SCOPES_CONFIG_DIR", "foobar", 1);

    RuntimeConfig c(TEST_DIR "/ConfigDir.ini");
    EXPECT_EQ("foobar", c.config_directory());
}

TEST_F(RuntimeConfigTest, trace_channels_env_var_override)
{
    setenv("UNITY_SCOPES_LOG_TRACECHANNELS", "ABC;XYZ;;DEF", 1);

    RuntimeConfig c(TEST_DIR "/Complete.ini");
    EXPECT_EQ((vector<string>{ "ABC", "XYZ", "DEF"}), c.trace_channels());
}

TEST_F(RuntimeConfigTest, exceptions)
{
    try
    {
        RuntimeConfig c(TEST_DIR "/BadMW.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_DIR "/BadMW.ini\": Illegal value for Default.Middleware: "
                     "\"Foo\": legal values are \"Zmq\" and \"REST\"",
                     e.what());
    }

    try
    {
        RuntimeConfig c(TEST_DIR "/BadReapExpiry.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_DIR "/BadReapExpiry.ini\": Illegal value (0) for "
                     "Reap.Expiry: value must be > 0",
                     e.what());
    }

    try
    {
        RuntimeConfig c(TEST_DIR "/BadReapInterval.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_DIR "/BadReapInterval.ini\": Illegal value (0) for "
                     "Reap.Interval: value must be > 0",
                     e.what());
    }

    try
    {
        unsetenv("HOME");

        RuntimeConfig c(TEST_DIR "/NoCacheDir.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_DIR "/NoCacheDir.ini\": No CacheDir configured and "
                     "failed to get default:\n    unity::ResourceException: RuntimeConfig::default_cache_directory(): "
                     "$HOME not set",
                     e.what());
    }

    try
    {
        unsetenv("HOME");

        RuntimeConfig c(TEST_DIR "/NoConfigDir.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_DIR "/NoConfigDir.ini\": No ConfigDir configured and "
                     "failed to get default:\n    unity::ResourceException: RuntimeConfig::default_config_directory(): "
                     "$HOME not set",
                     e.what());
    }

    try
    {
        unsetenv("HOME");

        RuntimeConfig c(TEST_DIR "/NoAppDir.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_DIR "/NoAppDir.ini\": "
                     "No AppDir configured and failed to get default:\n"
                     "    unity::ResourceException: RuntimeConfig::default_app_directory(): $HOME not set",
                     e.what());
    }
}
