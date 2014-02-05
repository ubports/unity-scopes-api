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
#include <unity/scopes/ScopeExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RuntimeConfig, basic)
{
    RuntimeConfig c("");
    EXPECT_EQ("Registry", c.registry_identity());
    EXPECT_EQ("Registry.ini", c.registry_configfile());
    EXPECT_EQ("Zmq", c.default_middleware());
}

TEST(RuntimeConfig, BadRegistryID)
{
    try
    {
        RuntimeConfig c("BadRegID.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"BadRegID.ini\": Illegal character in value for "
                     "Registry.Identity: \"Regi/stry\": identity cannot contain '/'",
                     e.what());
    }
}

TEST(RuntimeConfig, MiddlewareEmpty)
{
    try
    {
        RuntimeConfig c("MWEmpty.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"MWEmpty.ini\": Illegal empty value for Default.Middleware",
                     e.what());
    }
}

TEST(RuntimeConfig, BadMiddleware)
{
    try
    {
        RuntimeConfig c("BadMW.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: \"BadMW.ini\": Illegal value for Default.Middleware: "
                     "\"Foo\": legal values are \"Zmq\" and \"REST\"",
                     e.what());
    }
}
