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

#include <scopes/internal/ConfigBase.h>
#include <scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#include <gtest/gtest.h>
//#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.

using namespace std;
using namespace unity;
using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

class MyConfig : public ConfigBase
{
public:
    MyConfig(string const& configfile) :
        ConfigBase(configfile)
    {
    }

    void error()
    {
        throw_ex("error");
    }
};

TEST(ConfigBase, basic)
{
    MyConfig c("Test.ini");
    EXPECT_TRUE(c.parser().get());
}

TEST(ConfigBase, optional_string)
{
    MyConfig c("Test.ini");
    EXPECT_EQ("", c.get_optional_string("SomeGroup", "NoSuchKey"));
    EXPECT_EQ("", c.get_optional_string("SomeGroup", "Empty"));
}

TEST(ConfigBase, non_optional_string)
{
    MyConfig c("Test.ini");
    try
    {
        c.get_string("SomeGroup", "Empty");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: \"Test.ini\": Illegal empty value for Empty", e.to_string());
    }
}

TEST(ConfigBase, middleware)
{
    MyConfig c("Test.ini");
    EXPECT_EQ("Zmq", c.get_middleware("SomeGroup", "Zmq.Middleware"));
    EXPECT_EQ("REST", c.get_middleware("SomeGroup", "REST.Middleware"));
    try
    {
        c.get_middleware("SomeGroup", "Zmq.BadMiddleware");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: \"Test.ini\": Illegal value for Zmq.BadMiddleware: \"foo\": "
                  "legal values are \"Zmq\" and \"REST\"",
                  e.to_string());
    }
    try
    {
        c.get_middleware("SomeGroup", "REST.BadMiddleware");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: \"Test.ini\": Illegal value for REST.BadMiddleware: \"bar\": "
                  "legal values are \"Zmq\" and \"REST\"",
                  e.to_string());
    }
}

TEST(ConfigBase, throw_ex)
{
    MyConfig c("Test.ini");
    EXPECT_TRUE(c.parser().get());

    try
    {
        c.error();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::api::scopes::ConfigException: \"Test.ini\": error", e.to_string());
    }
}

TEST(ConfigBase, FileException)
{
    try
    {
        MyConfig c("no_such_file");
    }
    catch (FileException const& e)
    {
        EXPECT_EQ("unity::FileException: Could not load ini file no_such_file: No such file or directory (errno = 4)",
                  e.to_string());
    }
}
