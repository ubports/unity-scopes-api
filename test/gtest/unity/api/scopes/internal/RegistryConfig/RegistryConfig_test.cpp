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

#include <scopes/internal/RegistryConfig.h>
#include <unity/UnityExceptions.h>
#include <scopes/ScopeExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(RegistryConfig, basic)
{
    RegistryConfig c("Registry", "Registry.ini");
    EXPECT_EQ("Registry", c.identity());
    EXPECT_EQ("Zmq", c.mw_kind());
    EXPECT_EQ("ipc:///tmp/socket_for_registry", c.endpoint());
    EXPECT_EQ("Zmq.ini", c.mw_configfile());
}

TEST(RegistryConfig, RegistryIDEmpty)
{
    try
    {
        RegistryConfig c("", "Registry.ini");
        FAIL();
    }
    catch (InvalidArgumentException const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: Registry identity cannot be an empty string",
                  e.to_string());
    }
}

TEST(RegistryConfig, ScoperunnerRelativePath)
{
    try
    {
        RegistryConfig c("Registry", "ScoperunnerRelativePath.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: ScoperunnerRelativePath.ini: Scoperunner.Path "
                  "must be an absolute path",
                  e.to_string());
    }
}
