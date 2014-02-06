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

#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/ScopeExceptions.h>

#include <gtest/gtest.h>
#include <scope-api-testconfig.h>

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RuntimeImpl, basic)
{
    RuntimeImpl::UPtr rt = RuntimeImpl::create("testscope",
                                               TEST_BUILD_ROOT "/gtest/scopes/internal/RuntimeImpl/Runtime.ini");

    EXPECT_TRUE(rt->registry().get() != nullptr);
    EXPECT_TRUE(rt->factory());
    cerr << "reg: " << rt->registry_configfile() << endl;
    EXPECT_EQ(TEST_BUILD_ROOT "/gtest/scopes/internal/RuntimeImpl/Registry.ini", rt->registry_configfile());

    rt->destroy();

    try
    {
        rt->registry();
        FAIL();
    }
    catch (LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: registry(): Cannot obtain registry for already destroyed run time",
                     e.what());
    }

    rt->destroy();
    try
    {
        rt->factory();
        FAIL();
    }
    catch (LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: factory(): Cannot obtain factory for already destroyed run time",
                     e.what());
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
        EXPECT_STREQ("unity::scopes::ConfigException: Cannot instantiate run time for testscope, "
                     "config file: NoSuchFile.ini:\n"
                     "    unity::FileException: Could not load ini file NoSuchFile.ini: No such file or directory (errno = 4)",
                     e.what());
    }
}
