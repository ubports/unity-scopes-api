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

#include <scopes/internal/RegistryImpl.h>
#include <scopes/ScopeBase.h>
#include <scopes/internal/ScopeLoader.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#include <scope-api-testconfig.h>
#include "Counters.h"
#include "PerScopeVariables.h"

#include <thread>

using namespace std;
using namespace unity::scopes;
using namespace unity::scopes::internal;

namespace
{

char const* scope_lib = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libTestScope.so";
char const* bad_version_lib = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libBadVersion.so";
char const* no_destroy_lib = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libNoDestroy.so";
char const* null_return_lib = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libNullReturn.so";
char const* throw_unity_ex_from_start_lib
    = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libThrowUnityExFromStart.so";
char const* throw_unity_ex_from_stop_lib
    = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libThrowUnityExFromStop.so";
char const* throw_std_ex_from_stop_lib
    = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libThrowStdExFromStop.so";
char const* throw_unknown_ex_from_start_lib
    = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libThrowUnknownExFromStart.so";
char const* throw_unknown_ex_from_stop_lib
    = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libThrowUnknownExFromStop.so";
char const* scopeA
    = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libScopeA.so";
char const* scopeB
    = TEST_BUILD_ROOT "/gtest/scopes/internal/ScopeLoader/libScopeB.so";
}

// Need to make a dummy registry proxy because, if we pass a null shared_ptr to load(),
// load() throws an exception.

RegistryProxy registry(RegistryImpl::create(nullptr, (RuntimeImpl*)0x1));

// Basic test.

TEST(ScopeLoader, basic)
{
    reset_counters();

    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(0, num_destroy());

        EXPECT_EQ("testScope", sl->name());
        EXPECT_EQ(scope_lib, sl->libpath());
    }
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
    EXPECT_EQ(0, num_start());
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
        EXPECT_EQ("unity::InvalidArgumentException: Cannot load scope testScope: null registry proxy",
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
        sl->unload();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        boost::regex r("unity::ResourceException: Scope testScope: terminated due to exception in start\\(\\):\n"
                       "    unity::ResourceException: Scope testScope was compiled with major version 666 of the "
                       "Unity scopes run time. This version is incompatible with the current major version "
                       "\\([0-9]+\\)\\.");
        EXPECT_TRUE(boost::regex_match(e.to_string(), r));
    }
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_destroy());
    EXPECT_EQ(1, num_start());
    EXPECT_EQ(1, num_stop());
}

TEST(ScopeLoader, stop)
{
    reset_counters();

    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);

        // Check that calling stop on a stopped thread does nothing.
        sl->stop();
        EXPECT_EQ(0, num_start());
        EXPECT_EQ(0, num_stop());
        EXPECT_TRUE(sl->scope_base());  // Just so we get coverage

        sl->stop();
        EXPECT_EQ(0, num_start());
        EXPECT_EQ(0, num_stop());

        // Check transition to start state and that calling start more than once does nothing.
        sl->start();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(0, num_stop());

        sl->start();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(0, num_stop());

        // Call stop/start/stop and check counts.
        sl->stop();
        EXPECT_EQ(1, num_stop());
        sl->start();
        sl->stop();
        EXPECT_EQ(2, num_start());
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
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(0, num_stop());
    }
    EXPECT_EQ(1, num_start());
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
        EXPECT_EQ(0, num_destroy());
    }
}

// Same thing again, but this time with an explicit unload

TEST(ScopeLoader, null_return_unload)
{
    reset_counters();

    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", null_return_lib, registry);
        sl->unload();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::ResourceException: Scope testScope returned nullptr from unity_api_scope_create",
                  e.to_string());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(0, num_destroy());
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
            sl->unload();
            FAIL();
        }
        catch (unity::Exception const& e)
        {
            EXPECT_EQ("unity::ResourceException: Scope testScope: terminated due to exception in start():\n"
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
        try
        {
            sl->start();
            FAIL();
        }
        catch (unity::Exception const& e)
        {
            EXPECT_EQ("unity::ResourceException: Scope testScope: terminated due to exception in start():\n"
                      "    unity::LogicException: start failure",
                      e.to_string());
        }
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
            sl->unload();
            EXPECT_TRUE(sl->scope_base());
            FAIL();
        }
        catch (unity::Exception const& e)
        {
            EXPECT_EQ("unity::ResourceException: Scope testScope: terminated due to exception in start():\n"
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
        sl->stop();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_stop());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
        sl->unload();
        EXPECT_TRUE(sl->scope_base());
        FAIL();
    }
    catch (unity::ResourceException const& e)
    {
        EXPECT_EQ("unity::ResourceException: Scope testScope: terminated due to exception in stop():\n"
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
        sl->stop();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_stop());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
        sl->unload();
        EXPECT_TRUE(sl->scope_base());
        FAIL();
    }
    catch (unity::ResourceException const& e)
    {
        EXPECT_EQ("unity::ResourceException: Scope testScope: terminated due to exception in stop():\n"
                  "    stop failure",
                  e.to_string());
    }
}

// Test that things behave as expected if stop() throws an unknown exception.

TEST(ScopeLoader, throw_unknown_exception_from_stop)
{
    reset_counters();
    try
    {
        ScopeLoader::UPtr sl = ScopeLoader::load("testScope", throw_unknown_ex_from_stop_lib, registry);
        sl->start();
        sl->stop();
        EXPECT_EQ(1, num_start());
        EXPECT_EQ(1, num_stop());
        EXPECT_EQ(1, num_create());
        EXPECT_EQ(1, num_destroy());
        sl->unload();
        FAIL();
    }
    catch (unity::ResourceException const& e)
    {
        EXPECT_EQ("unity::ResourceException: Scope testScope: terminated due to exception in stop():\n"
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
    sl->stop();
    sl->unload();
    sl->unload();

    EXPECT_EQ(1, num_start());
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
    try
    {
        sl->unload();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::ResourceException: Scope testScope: terminated due to exception in stop():\n"
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
    sl->unload();
    try
    {
        sl->start();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::LogicException: Cannot start scope testScope in Finished state", e.to_string());
    }
}

// Test that stop() throws an exception after unload()

TEST(ScopeLoader, stop_after_unload_exception)
{
    reset_counters();

    ScopeLoader::UPtr sl = ScopeLoader::load("testScope", scope_lib, registry);
    sl->start();
    sl->unload();
    try
    {
        sl->stop();
        FAIL();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::LogicException: Cannot stop scope testScope in Finished state", e.to_string());
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
        sl->unload();
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
        EXPECT_EQ("unity::LogicException: Cannot start scope testScope in Failed state", e.to_string());
    }

    try
    {
        sl->stop();
    }
    catch (unity::Exception const& e)
    {
        EXPECT_EQ("unity::LogicException: Cannot stop scope testScope in Failed state", e.to_string());
    }
}

// Test that we can have more than one scope loaded without getting confused which one we are talking to.

TEST(ScopeLoader, two_scopes)
{
    reset_counters();
    clear_vars();

    ScopeLoader::UPtr slA = ScopeLoader::load("scopeA", scopeA, registry);
    slA->start();
    EXPECT_EQ(1, num_create());
    EXPECT_EQ(1, num_start());

    ScopeLoader::UPtr slB = ScopeLoader::load("scopeB", scopeB, registry);
    slB->start();
    EXPECT_EQ(2, num_create());
    EXPECT_EQ(2, num_start());

    EXPECT_EQ(0, get_scopeA_var());
    EXPECT_EQ(0, get_scopeB_var());

    slA->unload();

    EXPECT_EQ(1, get_scopeA_var());
    EXPECT_EQ(0, get_scopeB_var());

    slB->unload();

    EXPECT_EQ(1, get_scopeA_var());
    EXPECT_EQ(2, get_scopeB_var());
}
