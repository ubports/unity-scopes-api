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
        EXPECT_EQ("cachedir", c.cache_directory());
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
        EXPECT_EQ("cachedir", c.cache_directory());
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"" TEST_SRC_DIR "/NoConfigDir.ini\": No ConfigDir configured and "
                     "failed to get default:\n    unity::ResourceException: RuntimeConfig::default_config_directory(): "
                     "$HOME not set",
                     e.what());
    }
}
