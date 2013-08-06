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

#include <unity/api/scopes/internal/MiddlewareFactory.h>
#include <unity/api/scopes/ScopeExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(MiddlewareFactory, basic)
{
    MiddlewareFactory f("Factory.ini");
    EXPECT_EQ(nullptr, f.find("nosuchscope", "Ice"));
    MiddlewareBase::SPtr not_found;
    MiddlewareBase::SPtr mw = f.find("testscope", "Ice");
    EXPECT_EQ(not_found, mw);
    mw = f.find("testscope", "NoSuchMiddleware");
    EXPECT_EQ(not_found, mw);
}

TEST(MiddlewareFactory, BadKind)
{
    try
    {
        MiddlewareFactory f("Factory.ini");
        f.create("somescope", "NoSuchMiddleware", "Ice.Config");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: Invalid middleware kind: NoSuchMiddleware", e.to_string());
    }
}

TEST(MiddlewareFactory, BadIniFile)
{
    try
    {
        MiddlewareFactory f("NoSuchFile.ini");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: cannot instantiate MiddlewareFactory: "
                  "config file: NoSuchFile.ini:\n"
                  "    unity::FileException: Could not load ini file NoSuchFile.ini: "
                  "No such file or directory (errno = 4)",
                  e.to_string());
    }
}
