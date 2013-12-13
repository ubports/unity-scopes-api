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

#include <scopes/internal/RuntimeConfig.h>
#include <scopes/ScopeExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(RuntimeConfig, basic)
{
    RuntimeConfig c("");
    EXPECT_EQ("Registry", c.registry_identity());
    EXPECT_EQ("Registry.ini", c.registry_configfile());
    EXPECT_EQ("Zmq", c.default_middleware());
    EXPECT_EQ("Factory.ini", c.factory_configfile());
}

TEST(RuntimeConfig, RegistryIDEmpty)
{
    try
    {
        RuntimeConfig c("RegIDEmpty.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: \"RegIDEmpty.ini\": Illegal empty value for Registry.Identity",
                  e.to_string());
    }
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
        EXPECT_EQ("unity::api::scopes::ConfigException: \"BadRegID.ini\": Illegal character in value for "
                  "Registry.Identity: \"Regi/stry\": identity cannot contain '/'",
                  e.to_string());
    }
}

TEST(RuntimeConfig, RegistryFileEmpty)
{
    try
    {
        RuntimeConfig c("RegFileEmpty.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: \"RegFileEmpty.ini\": Illegal empty value for Registry.ConfigFile",
                  e.to_string());
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
        EXPECT_EQ("unity::api::scopes::ConfigException: \"MWEmpty.ini\": Illegal empty value for Default.Middleware",
                  e.to_string());
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
        EXPECT_EQ("unity::api::scopes::ConfigException: \"BadMW.ini\": Illegal value for Default.Middleware: "
                  "\"Foo\": legal values are \"Zmq\" and \"REST\"",
                  e.to_string());
    }
}

TEST(RuntimeConfig, FactoryFileEmpty)
{
    try
    {
        RuntimeConfig c("FacFileEmpty.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: \"FacFileEmpty.ini\": Illegal empty value for Factory.ConfigFile",
                  e.to_string());
    }
}
