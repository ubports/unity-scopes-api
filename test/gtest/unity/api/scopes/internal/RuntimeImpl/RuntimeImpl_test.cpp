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

#include <unity/api/scopes/internal/RuntimeImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/api/scopes/ScopeExceptions.h>

#include <gtest/gtest.h>

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(RuntimeImpl, basic)
{
    RuntimeImpl::UPtr rt = RuntimeImpl::create("testscope");

    EXPECT_TRUE(rt->registry().get() != nullptr);
    EXPECT_TRUE(rt->factory());
    EXPECT_EQ("Registry.ini", rt->registry_configfile());

    rt->destroy();

    try
    {
        rt->registry();
        FAIL();
    }
    catch (LogicException const& e)
    {
        EXPECT_EQ("unity::LogicException: registry(): Cannot obtain registry for already destroyed run time",
                  e.to_string());
    }

    rt->destroy();
    try
    {
        rt->factory();
        FAIL();
    }
    catch (LogicException const& e)
    {
        EXPECT_EQ("unity::LogicException: factory(): Cannot obtain factory for already destroyed run time",
                  e.to_string());
    }
}

TEST(RuntimeImpl, error)
{
    try
    {
        RuntimeImpl::UPtr rt = RuntimeImpl::create("testscope", "NoSuchFile.ini");
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: cannot instantiate run time for testscope, "
                  "config file: NoSuchFile.ini:\n"
                  "    unity::FileException: Could not load ini file NoSuchFile.ini: No such file or directory (errno = 4)",
                  e.to_string());
    }
}
