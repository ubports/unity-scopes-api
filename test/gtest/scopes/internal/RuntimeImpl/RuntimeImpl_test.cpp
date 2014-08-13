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

class Waiter
{
public:
    Waiter()
        : ready_(false)
    {
    };

    void set_ready() noexcept
    {
        lock_guard<mutex> lock(m_);
        ready_ = true;
        cond_.notify_all();
    }

    void wait_until_ready()
    {
        unique_lock<mutex> lock(m_);
        cond_.wait(lock, [this]{ return ready_; });
    }

private:
    mutex m_;
    condition_variable cond_;
    bool ready_;
};

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
            rt->run_scope(&testscope, rt_ini_file, scope_ini_file);
        };
        auto fut = std::async(launch::async, thread_func);

        try
        {
            fut.get();
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
        Waiter waiter;
        auto cb = [&waiter]{ waiter.set_ready(); };

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope(TestScope::ThrowFromRun);
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, cb]
        {
            rt->run_scope(&testscope, rt_ini_file, scope_ini_file, cb);
        };
        auto fut = std::async(launch::async, thread_func);

        waiter.wait_until_ready();
        rt->destroy();

        try
        {
            fut.get();
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
        Waiter waiter;
        auto cb = [&waiter]{ waiter.set_ready(); };

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope(TestScope::ThrowFromStop);
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, cb]
        {
            rt->run_scope(&testscope, rt_ini_file, scope_ini_file, cb);
        };
        auto fut = std::async(launch::async, thread_func);

        // Don't destroy the run time until after the scope has finished initializing.
        waiter.wait_until_ready();
        rt->destroy();

        try
        {
            fut.get();
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

        Waiter waiter;
        auto cb = [&waiter]{ waiter.set_ready(); };

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope;
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, cb]
        {
            rt->run_scope(&testscope, rt_ini_file, scope_ini_file, cb);
        };
        auto fut = std::async(launch::async, thread_func);

        // Directories are not available before start() is called on the scope.
        testscope.wait_until_started();

        EXPECT_EQ(TEST_DIR, testscope.scope_directory());

        string tmpdir = "/run/user/" + to_string(geteuid()) + "/scopes/unconfined/TestScope";
        EXPECT_EQ(tmpdir, testscope.tmp_directory());

        EXPECT_EQ(TEST_DIR "/cache_dir/unconfined/TestScope", testscope.cache_directory());

        // Don't shut down the run time until after the scope has initialized.
        waiter.wait_until_ready();
        rt->destroy();

        fut.wait();
    }

    {
        // Same test again, but, this time, we remove write permission from the
        // scope's directory. This cons the run time into thinking that the scope is confined.

        char const* dir_path = TEST_DIR "/cache_dir/unconfined";

        // Make sure we restore previous permissions if something throws.
        auto restore_perms = [](char const* path){ ::chmod(path, 0700); };
        util::ResourcePtr<char const*, decltype(restore_perms)> permission_guard(dir_path, restore_perms);
        ::chmod(dir_path, 0000);

        Waiter waiter;
        auto cb = [&waiter]{ waiter.set_ready(); };

        auto rt = move(RuntimeImpl::create("TestScope", rt_ini_file));
        TestScope testscope;
        auto thread_func = [&rt, &testscope, &rt_ini_file, &scope_ini_file, cb]
        {
            rt->run_scope(&testscope, rt_ini_file, scope_ini_file, cb);
        };
        auto fut = std::async(launch::async, thread_func);

        // Directories are not available before start() is called on the scope.
        testscope.wait_until_started();

        EXPECT_EQ(TEST_DIR, testscope.scope_directory());

        string tmpdir = "/run/user/" + to_string(geteuid()) + "/scopes/leaf-net/TestScope";
        EXPECT_EQ(tmpdir, testscope.tmp_directory());

        EXPECT_EQ(TEST_DIR "/cache_dir/leaf-net/TestScope", testscope.cache_directory());

        // Don't shut down the run time until after the scope has initialized.
        waiter.wait_until_ready();
        rt->destroy();

        fut.wait();
    }
}
