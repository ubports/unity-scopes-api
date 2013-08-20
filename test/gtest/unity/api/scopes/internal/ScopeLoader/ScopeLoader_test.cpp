/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/RegistryImpl.h>
#include <unity/api/scopes/ScopeBase.h>
#include <unity/api/scopes/internal/ScopeLoader.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#include <scope-api-testconfig.h>
#include "Counters.h"
#include "PerScopeVariables.h"

#include <time.h>

using namespace std;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

namespace
{

char const* scope_lib = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libTestScope.so";
char const* bad_version_lib = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libBadVersion.so";
char const* no_destroy_lib = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libNoDestroy.so";
char const* null_return_lib = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libNullReturn.so";
char const* throw_unity_ex_from_run_lib
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libThrowUnityExFromRun.so";
char const* throw_unity_ex_from_start_lib
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libThrowUnityExFromStart.so";
char const* throw_unity_ex_from_stop_lib
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libThrowUnityExFromStop.so";
char const* throw_std_ex_from_stop_lib
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libThrowStdExFromStop.so";
char const* throw_unknown_ex_from_start_lib
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libThrowUnknownExFromStart.so";
char const* throw_unknown_ex_from_stop_lib
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libThrowUnknownExFromStop.so";
char const* scopeA
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libScopeA.so";
char const* scopeB
    = TEST_BUILD_ROOT "/gtest/unity/api/scopes/internal/ScopeLoader/libScopeB.so";
}

// Function to sleep for specified number of milliseconds. We use this
// before we test for the counter values because the thread responds
// asynchronously to our commands and may take a little while to catch up.
// This is particularly true when testing under valgrind, where things
// are slower.

void wait(long millisec = 100)
{
    struct timespec tv;
    tv.tv_sec = millisec / 1000;
    tv.tv_nsec = millisec % 1000 * 1000000;
    nanosleep(&tv, nullptr);
}

// Need to make a dummy registry proxy because, if we pass a null shared_ptr to load(),
// load() throws an exception.

RegistryProxy registry(RegistryImpl::create(nullptr));

// Basic test.

TEST(ScopeLoader, basic)
{
    reset_counters();

    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);
        wait();
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(0, num_destroy());

        EXPECT_EQ("testScope", sl->name());
        EXPECT_EQ(scope_lib, sl->libpath());
    }
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
    EXPECT_EQ(0, num_start());
    EXPECT_EQ(0, num_run());
    EXPECT_EQ(0, num_stop());
}

// Check that we get an exception for a null registry proxy.

TEST(ScopeLoader, null_registry)
{
    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, nullptr);
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::InvalidArgumentException: Cannot load scope \"testScope\": null registry proxy",
                  e.to_string());
    }
}

TEST(ScopeLoader, version_mismatch)
{
    reset_counters();

    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", bad_version_lib, registry);
        sl->start();
        wait();
        sl->unload();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        boost::regex r("unity::ResourceException: scope testScope: terminated due to exception:\n"
                       "    unity::ResourceException: Scope testScope was compiled with major version 666 of the "
                       "Unity scopes run time. This version is incompatible with the current major version "
                       "\\([0-9]+\\)\\.");
        EXPECT_TRUE(boost::regex_match(e.to_string(), r));
    }
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
    EXPECT_EQ(1, num_start());
    EXPECT_EQ(0, num_run());
    EXPECT_EQ(0, num_stop());
}

TEST(ScopeLoader, stop)
{
    reset_counters();

    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);

        // Check that calling stop on a stopped thread does nothing.
        sl->stop();
        wait();
        EXPECT_EQ(0, num_start());
        EXPECT_EQ(0, num_stop());
        EXPECT_TRUE(sl->scope_base());  // Just so we get coverage

        sl->stop();
        wait();
        EXPECT_EQ(0, num_start());
        EXPECT_EQ(0, num_stop());

        // Check transition to start state and that calling start more than once does nothing.
        sl->start();
        wait();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_run());
        EXPECT_EQ(0, num_stop());

        sl->start();
        wait();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_run());
        EXPECT_EQ(0, num_stop());

        // Call stop/start/stop and check counts. We need to wait a little between calls to allow
        // the thread to catch up.
        sl->stop();
        wait();
        EXPECT_EQ(1, num_stop());
        sl->start();
        wait();
        sl->stop();
        wait();
        EXPECT_EQ(2, num_start());
        EXPECT_EQ(2, num_run());
        EXPECT_EQ(2, num_stop());
    }
}

// Test that letting the ScopeLoader go out of scope while the scope is running shuts down the scope.

TEST(ScopeLoader, unload_while_started)
{
    reset_counters();

    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);

        sl->start();
        wait();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(0, num_stop());
    }
    wait();
    EXPECT_EQ(1, num_start());
    EXPECT_EQ(1, num_run());
    EXPECT_EQ(1, num_stop());
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
}

// Test that things behave as expected if the library can't be loaded.

TEST(ScopeLoader, no_library)
{
    reset_counters();

    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", "no_such_lib", registry);
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::ResourceException: no_such_lib: cannot open shared object file: No such file or directory",
                  e.to_string());
        EXPECT_EQ(0, num_create());
        EXPECT_EQ(0, num_destroy());
    }
}

// Test that things behave as expected if the create or destroy function isn't found.

TEST(ScopeLoader, no_load)
{
    reset_counters();

    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", no_destroy_lib, registry);
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        boost::regex r("unity::ResourceException: .*/libNoDestroy.so: undefined symbol: unity_api_scope_destroy");
        EXPECT_TRUE(boost::regex_match(e.to_string(), r));
        EXPECT_EQ(0, num_create());
        EXPECT_EQ(0, num_destroy());
    }
}

// Test that things behave as expected if the create function returns nullptr.

TEST(ScopeLoader, null_return)
{
    reset_counters();

    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", null_return_lib, registry);
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::ResourceException: Scope testScope returned nullptr from unity_api_scope_create",
                  e.to_string());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
    }
}

// Same thing again, but this time with an explicit unload

TEST(ScopeLoader, null_return_unload)
{
    reset_counters();

    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", null_return_lib, registry);
        wait();
        sl->unload();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::ResourceException: Scope testScope returned nullptr from unity_api_scope_create",
                  e.to_string());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
    }
}

// Test that things behave as expected if start() throws a Unity exception.

TEST(ScopeLoader, throw_unity_exception_from_start)
{
    reset_counters();
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unity_ex_from_start_lib, registry);
        try
        {
            sl->start();
            wait();
            sl->unload();
            wait();
            FAIL();
        }
        catch (unity::Exception const& e)
        {
            EXPECT_EQ("unity::ResourceException: scope testScope: terminated due to exception:\n"
                      "    unity::LogicException: start failure",
                      e.to_string());
        }
    }
    EXPECT_EQ(0, num_stop());
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
}

// Same again, but without an explicit unload, just so we get coverage in the destructor.

TEST(ScopeLoader, throw_unity_exception_from_start_no_unload)
{
    reset_counters();
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unity_ex_from_start_lib, registry);
        sl->start();
        wait();
    }
    EXPECT_EQ(0, num_stop());
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
}

// Test that things behave as expected if start() throws an unknown exception.

TEST(ScopeLoader, throw_unknown_exception_from_start)
{
    reset_counters();
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unknown_ex_from_start_lib, registry);
        try
        {
            sl->start();
            wait();
            sl->unload();
            wait();
            EXPECT_TRUE(sl->scope_base());
            FAIL();
        }
        catch (unity::Exception const& e)
        {
            EXPECT_EQ("unity::ResourceException: scope testScope: terminated:\n"
                      "    unknown exception",
                      e.to_string());
        }
    }
    EXPECT_EQ(0, num_stop());
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
}

// Test that things behave as expected if stop() throws a Unity exception.

TEST(ScopeLoader, throw_unity_exception_from_stop)
{
    reset_counters();
    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unity_ex_from_stop_lib, registry);
        sl->start();
        wait();
        sl->stop();
        wait();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_run());
        EXPECT_EQ(1, num_stop());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
        sl->unload();
        EXPECT_TRUE(sl->scope_base());
        FAIL();
    }
    catch (unity::ResourceException const& e)
    {
        EXPECT_EQ("unity::ResourceException: scope testScope: terminated due to exception:\n"
                  "    unity::LogicException: stop failure",
                  e.to_string());
    }
}

// Test that things behave as expected if stop() throws a std exception.

TEST(ScopeLoader, throw_std_exception_from_stop)
{
    reset_counters();
    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_std_ex_from_stop_lib, registry);
        sl->start();
        wait();
        sl->stop();
        wait();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_run());
        EXPECT_EQ(1, num_stop());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
        sl->unload();
        EXPECT_TRUE(sl->scope_base());
        FAIL();
    }
    catch (unity::ResourceException const& e)
    {
        EXPECT_EQ("unity::ResourceException: scope testScope: terminated due to exception:\n"
                  "    stop failure",
                  e.to_string());
    }
}

// Test that things behave as expected if run() throws a Unity exception.

TEST(ScopeLoader, throw_unity_exception_from_run)
{
    reset_counters();
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unity_ex_from_run_lib, registry);
        try
        {
            sl->start();
            wait();
            sl->unload();
            wait();
            FAIL();
        }
        catch (unity::Exception const& e)
        {
            EXPECT_EQ("unity::ResourceException: Scope testScope: exception in run():\n"
                      "    unity::LogicException: run failure",
                      e.to_string());
        }
    }

    EXPECT_EQ(1, num_start());
    EXPECT_EQ(1, num_run());
    EXPECT_EQ(1, num_stop());
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
}

// Test that things behave as expected if stop() throws an unknown exception.

TEST(ScopeLoader, throw_unknown_exception_from_stop)
{
    reset_counters();
    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unknown_ex_from_stop_lib, registry);
        sl->start();
        wait();
        sl->stop();
        wait();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_run());
        EXPECT_EQ(1, num_stop());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
        sl->unload();
        FAIL();
    }
    catch (unity::ResourceException const& e)
    {
        EXPECT_EQ("unity::ResourceException: scope testScope: terminated:\n"
                  "    unknown exception",
                  e.to_string());
    }
}

// Test that things behave as expected if unload is called more than once

TEST(ScopeLoader, unload)
{
    reset_counters();

    ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);
    sl->start();
    wait();
    sl->stop();
    sl->unload();
    wait();
    sl->unload();
    wait();

    EXPECT_EQ(1, num_start());
    EXPECT_EQ(1, num_run());
    EXPECT_EQ(1, num_stop());
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
}

// Test that things behave as expected if unload is called while the scope is running and stop() throws

TEST(ScopeLoader, unload_stop_exception)
{
    reset_counters();

    ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unity_ex_from_stop_lib, registry);
    sl->start();
    wait();
    try
    {
        sl->unload();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::ResourceException: scope testScope: terminated due to exception:\n"
                  "    unity::LogicException: stop failure",
                  e.to_string());
    }

    // Call unload again to make sure that the second time, it doesn't throw.
    try
    {
        sl->unload();
    }
    catch (...)
    {
        FAIL();
    }
}

// Test that start() throws an exception after unload()

TEST(ScopeLoader, restart_exception)
{
    reset_counters();

    ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);
    sl->start();
    wait();
    sl->unload();
    wait();
    try
    {
        sl->start();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::LogicException: Cannot start scope \"testScope\" in Finished state", e.to_string());
    }
}

// Test that start() and stop() throw for scope in Failed state

TEST(ScopeLoader, restart_failed)
{
    reset_counters();

    ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unity_ex_from_start_lib, registry);
    try
    {
        sl->start();
        wait();
        sl->unload();
        wait();
        FAIL();
    }
    catch (unity::Exception const&)
    {
    }

    try
    {
        sl->start();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::LogicException: Cannot start scope \"testScope\" in Failed state", e.to_string());
    }

    try
    {
        sl->stop();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::LogicException: Cannot stop scope \"testScope\" in Failed state", e.to_string());
    }
}

// Test that we can have more than one scope loaded without getting confused which one we are talking to.

TEST(ScopeLoader, two_scopes)
{
    reset_counters();
    clear_vars();

    ScopeLoader::UPtr slA = ScopeLoader::load("scopeA", scopeA, registry);
    slA->start();
    wait();
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_start());

    ScopeLoader::UPtr slB = ScopeLoader::load("scopeB", scopeB, registry);
    slB->start();
    wait();
    EXPECT_EQ(2, num_create());
    EXPECT_EQ(2, num_start());

    EXPECT_EQ(0, get_scopeA_var());
    EXPECT_EQ(0, get_scopeB_var());

    slA->unload();
    wait();

    EXPECT_EQ(1, get_scopeA_var());
    EXPECT_EQ(0, get_scopeB_var());

    slB->unload();
    wait();

    EXPECT_EQ(1, get_scopeA_var());
    EXPECT_EQ(2, get_scopeB_var());
}
