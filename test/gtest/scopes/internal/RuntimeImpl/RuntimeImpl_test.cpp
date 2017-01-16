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
#include <unity/util/ResourcePtr.h>

#include "TestScope.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <fstream>

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

TEST(RuntimeImpl, exceptions)
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

    string const rt_ini_file = TEST_DIR "/Runtime.ini";
    string const scope_ini_file = TEST_DIR "/TestScope.ini";

    {
        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope(TestScope::ThrowFromStart);
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file]
        {
            rt->run_scope(&testscope, scope_ini_file);
        };
        auto thread_done = std::async(launch::async, thread_func);

        try
        {
            thread_done.get();
            FAIL();
        }
        catch (std::exception const& e)
        {
            EXPECT_STREQ("unity::ResourceException: Scope TestScope: exception from start():\n"
                         "    unity::ResourceException: Can't start",
                         e.what());
        }
    }

    {
        std::promise<void> promise;
        auto initialized = promise.get_future();

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope(TestScope::ThrowFromRun);
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, &promise]
        {
            rt->run_scope(&testscope, scope_ini_file, move(promise));
        };
        auto thread_done = std::async(launch::async, thread_func);

        // Don't destroy the run time until after the scope has finished initializing.
        initialized.wait();
        rt->destroy();

        try
        {
            thread_done.get();
            FAIL();
        }
        catch (std::exception const& e)
        {
            EXPECT_STREQ("unity::ResourceException: Scope TestScope: exception from run():\n"
                         "    unity::ResourceException: Can't run",
                         e.what());
        }
    }

    {
        std::promise<void> promise;
        auto initialized = promise.get_future();

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope(TestScope::ThrowFromStop);
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, &promise]
        {
            rt->run_scope(&testscope, scope_ini_file, move(promise));
        };
        auto thread_done = std::async(launch::async, thread_func);

        // Don't destroy the run time until after the scope has finished initializing.
        initialized.wait();
        rt->destroy();

        try
        {
            thread_done.get();
            FAIL();
        }
        catch (std::exception const& e)
        {
            EXPECT_STREQ("unity::ResourceException: Scope TestScope: exception from stop():\n"
                         "    unity::ResourceException: Can't stop",
                         e.what());
        }
    }
}

TEST(RuntimeImpl, directories)
{
    string const rt_ini_file = TEST_DIR "/Runtime.ini";
    string const scope_ini_file = TEST_DIR "/TestScope.ini";

    {
        boost::filesystem::remove_all(TEST_DIR "/cache_dir/unconfined");
        boost::filesystem::remove_all(TEST_DIR "/cache_dir/leaf-net");

        std::promise<void> promise;
        auto initialized = promise.get_future();

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope;
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, &promise]
        {
            rt->run_scope(&testscope, scope_ini_file, move(promise));
        };
        auto thread_done = std::async(launch::async, thread_func);

        // Directories are not available before start() is called on the scope.
        testscope.wait_until_started();

        EXPECT_EQ(TEST_DIR, testscope.scope_directory());

        string tmpdir = "/run/user/" + to_string(geteuid()) + "/scopes/unconfined/TestScope";
        EXPECT_EQ(tmpdir, testscope.tmp_directory());

        EXPECT_EQ(TEST_DIR "/cache_dir/unconfined/TestScope", testscope.cache_directory());
        EXPECT_EQ(TEST_DIR "/app_dir/TestScope", testscope.app_directory());

        // Don't destroy the run time until after the scope has finished initializing.
        initialized.wait();
        rt->destroy();

        thread_done.wait();
    }

    {
        // Check that scopes that share an app dir with an app (because
        // the scope is packaged with the app in a single click package)
        // return the correct path for cache and app dir.

        string const scope_ini_file = TEST_DIR "/TestScope_TestScope.ini";

        std::promise<void> promise;
        auto initialized = promise.get_future();

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope;
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, &promise]
        {
            rt->run_scope(&testscope, scope_ini_file, move(promise));
        };
        auto thread_done = std::async(launch::async, thread_func);

        // Directories are not available before start() is called on the scope.
        testscope.wait_until_started();

        EXPECT_EQ(TEST_DIR, testscope.scope_directory());

        string tmpdir = "/run/user/" + to_string(geteuid()) + "/scopes/unconfined/TestScope";
        EXPECT_EQ(tmpdir, testscope.tmp_directory());

        EXPECT_EQ(TEST_DIR "/cache_dir/unconfined/TestScope", testscope.cache_directory());
        EXPECT_EQ(TEST_DIR "/app_dir/TestScope", testscope.app_directory());

        // Don't destroy the run time until after the scope has finished initializing.
        initialized.wait();
        rt->destroy();

        thread_done.wait();
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ifstream la("/proc/loadavg");
    string avg[3];
    la >> avg[0] >> avg[1] >> avg[2];
    cerr << "load average: " << avg[0] << " " << avg[1] << " " << avg[2] << endl;

    return RUN_ALL_TESTS();
}
