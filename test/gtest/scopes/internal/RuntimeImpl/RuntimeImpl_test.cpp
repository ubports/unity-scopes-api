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
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include "TestScope.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>

#include <fcntl.h>

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RuntimeImpl, basic)
{
    // Make sure we start out with a clean slate.
    boost::filesystem::remove_all(TEST_DIR "/cache_dir");

    RuntimeImpl::UPtr rt = RuntimeImpl::create("TestScope", TEST_DIR "/Runtime.ini");

    EXPECT_TRUE(rt->registry().get() != nullptr);
    EXPECT_TRUE(rt->registry()->identity() == "Registry");
    EXPECT_TRUE(rt->factory());
    EXPECT_EQ(TEST_DIR "/Registry.ini", rt->registry_configfile());

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
        RuntimeImpl::UPtr rt = RuntimeImpl::create("TestScope", "NoSuchFile.ini");
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: Cannot instantiate run time for TestScope, "
                     "config file: NoSuchFile.ini:\n"
                     "    unity::FileException: Could not load ini file NoSuchFile.ini: No such file or directory (errno = 4)",
                     e.what());
    }
}

void testscope_thread(Runtime::SPtr const& rt, TestScope* testscope, string const& runtime_ini_file)
{
    rt->run_scope(testscope, runtime_ini_file, TEST_DIR "/TestScope.ini");
}

TEST(RuntimeImpl, directories)
{

    {
        Runtime::SPtr rt = move(Runtime::create_scope_runtime("TestScope", TEST_DIR "/Runtime.ini"));
        TestScope testscope;
        std::thread testscope_t(testscope_thread, rt, &testscope, TEST_DIR "/Runtime.ini");

        // Give thread some time to start up.
        this_thread::sleep_for(chrono::milliseconds(200));

        EXPECT_EQ(TEST_DIR, testscope.scope_directory());

        string tmpdir = "/run/user/" + to_string(geteuid()) + "/scopes/unconfined/TestScope";
        EXPECT_EQ(tmpdir, testscope.tmp_directory());

        EXPECT_EQ(TEST_DIR "/cache_dir/unconfined/TestScope", testscope.cache_directory());

        rt->destroy();
        testscope_t.join();
    }
}
