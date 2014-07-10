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

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RuntimeImpl, basic)
{
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

TEST(RuntimeImpl, defaults)
{
}

void testscope_thread(Runtime::SPtr const& rt, TestScope* testscope, string const& runtime_ini_file)
{
    cerr << "thread: calling run_scope" << endl;
    rt->run_scope(testscope, runtime_ini_file, TEST_DIR "/TestScope.ini");
    cerr << "thread: done calling run_scope" << endl;
}

TEST(RuntimeImpl, env_vars)
{
    // Make sure we start out with a clean slate.
    boost::filesystem::remove_all(TEST_DIR "/cache_dir");

    {
        // Try with PATH and LD_LIBRARY_PATH not set.
        setenv("LD_LIBRARY_PATH", "", 1);
        setenv("PATH", "", 1);

        Runtime::SPtr rt = move(Runtime::create_scope_runtime("TestScope", TEST_DIR "/Runtime.ini"));
        TestScope testscope;
        std::thread testscope_t(testscope_thread, rt, &testscope, TEST_DIR "/Runtime.ini");

        // Give thread some time to start up.
        this_thread::sleep_for(chrono::milliseconds(200));

        auto env_vars = testscope.env_vars();
        EXPECT_EQ(string("/run/user/") + to_string(geteuid()), env_vars["XDG_RUNTIME_DIR"]);
        EXPECT_EQ("/tmp", env_vars["TMPDIR"]);
        cerr << "data: " << env_vars["XDG_DATA_HOME"] << endl;
        EXPECT_EQ(TEST_DIR, env_vars["XDG_CONFIG_HOME"]);
        EXPECT_EQ("1", env_vars["UBUNTU_APPLICATION_ISOLATION"]);
        auto var = env_vars["LD_LIBRARY_PATH"];
        EXPECT_TRUE(boost::starts_with(var, string(TEST_DIR) + ":" + TEST_DIR + "/lib"));
        var = env_vars["PATH"];
        EXPECT_TRUE(boost::starts_with(var, string(TEST_DIR) + ":" + TEST_DIR + "/bin"));

        rt->destroy();
        testscope_t.join();
    }

    {
        // Again with PATH and LD_LIBRARY_PATH set to something, so we get coverage,
        // and with the cache directory created, also for coverage.
        setenv("LD_LIBRARY_PATH", "/lib", 1);
        setenv("PATH", "/bin", 1);
        ::mkdir(TEST_DIR "/cache_dir", 0700);
        ::mkdir(TEST_DIR "/cache_dir/leaf-net", 0700);
        ::mkdir(TEST_DIR "/cache_dir/leaf-net/TestScope", 0700);

        Runtime::SPtr rt = move(Runtime::create_scope_runtime("TestScope", TEST_DIR "/Runtime.ini"));
        TestScope testscope;
        std::thread testscope_t(testscope_thread, rt, &testscope, TEST_DIR "/Runtime.ini");

        // Give thread some time to start up.
        this_thread::sleep_for(chrono::milliseconds(200));

        auto env_vars = testscope.env_vars();
        EXPECT_EQ("1", env_vars["UBUNTU_APPLICATION_ISOLATION"]);
        auto var = env_vars["LD_LIBRARY_PATH"];
        EXPECT_TRUE(boost::starts_with(var, string(TEST_DIR) + ":" + TEST_DIR + "/lib"));
        var = env_vars["PATH"];
        EXPECT_TRUE(boost::starts_with(var, string(TEST_DIR) + ":" + TEST_DIR + "/bin"));
        EXPECT_EQ(string(TEST_DIR) + "/cache_dir/leaf-net/TestScope", env_vars["TMPDIR"]);

        rt->destroy();
        testscope_t.join();
    }
}
