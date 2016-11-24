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

#include <unity/scopes/internal/RegistryConfig.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/ScopeExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RegistryConfig, basic)
{
    RegistryConfig c("Registry", TEST_REGISTRY_PATH);
    EXPECT_EQ("Registry", c.identity());
    EXPECT_EQ("Zmq", c.mw_kind());
    EXPECT_EQ("Zmq.ini", c.mw_configfile());
    EXPECT_EQ(3000, c.process_timeout());
}

TEST(RegistryConfig, RegistryIDEmpty)
{
    RegistryConfig c("", TEST_REGISTRY_PATH);
}

TEST(RegistryConfig, HomeNotSet)
{
    try
    {
        putenv(const_cast<char*>("HOME="));
        RegistryConfig c("Registry", TEST_REGISTRY_PATH);
        FAIL();
    }
    catch (ConfigException const& e)
    {
        boost::regex r( "unity::scopes::ConfigException: .*: No Click.InstallDir "
                        "configured and \\$HOME not set");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }
}

TEST(RegistryConfig, ScoperunnerRelativePath)
{
    try
    {
        putenv(const_cast<char*>("HOME=/tmp"));
        RegistryConfig c("Registry", "ScoperunnerRelativePath.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: ScoperunnerRelativePath.ini: Scoperunner.Path "
                     "must be an absolute path",
                     e.what());
    }
}
