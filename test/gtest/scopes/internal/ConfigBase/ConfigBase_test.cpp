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

#include <unity/scopes/internal/ConfigBase.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;

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

const string TEST_INI = TEST_DIR "/Test.ini";

TEST(ConfigBase, basic)
{
    MyConfig c(TEST_INI);
    EXPECT_TRUE(c.parser().get());
}

TEST(ConfigBase, optional_string)
{
    MyConfig c(TEST_INI);
    EXPECT_EQ("", c.get_optional_string("SomeGroup", "NoSuchKey"));
    EXPECT_EQ("", c.get_optional_string("SomeGroup", "Empty"));
}

TEST(ConfigBase, non_optional_string)
{
    MyConfig c(TEST_INI);
    try
    {
        c.get_string("SomeGroup", "Empty");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::scopes::ConfigException: \"" + TEST_INI + "\": Illegal empty value for Empty", e.what());
    }
}

TEST(ConfigBase, middleware)
{
    MyConfig c(TEST_INI);
    EXPECT_EQ("Zmq", c.get_middleware("SomeGroup", "Zmq.Middleware"));
    EXPECT_EQ("REST", c.get_middleware("SomeGroup", "REST.Middleware"));
    try
    {
        c.get_middleware("SomeGroup", "Zmq.BadMiddleware");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::scopes::ConfigException: \"" + TEST_INI + "\": Illegal value for Zmq.BadMiddleware: \"foo\": "
                  "legal values are \"Zmq\" and \"REST\"",
                  e.what());
    }
    try
    {
        c.get_middleware("SomeGroup", "REST.BadMiddleware");
        FAIL();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::scopes::ConfigException: \"" + TEST_INI + "\": Illegal value for REST.BadMiddleware: \"bar\": "
                  "legal values are \"Zmq\" and \"REST\"",
                  e.what());
    }
}

TEST(ConfigBase, throw_ex)
{
    MyConfig c(TEST_INI);
    EXPECT_TRUE(c.parser().get());

    try
    {
        c.error();
    }
    catch (ConfigException const& e)
    {
        EXPECT_EQ("unity::scopes::ConfigException: \"" + TEST_INI + "\": error", e.what());
    }
}

TEST(ConfigBase, FileException)
{
    try
    {
        MyConfig c("wrong_extension.x");
    }
    catch (ConfigException const& e)
    {
        EXPECT_STREQ("unity::scopes::ConfigException: invalid config file name: \"wrong_extension.x\": "
                     "missing .ini extension",
                     e.what());
    }

    try
    {
        MyConfig c("no_such_file.ini");
    }
    catch (FileException const& e)
    {
        EXPECT_STREQ("unity::FileException: Could not load ini file no_such_file.ini: No such file or directory (errno = 4)",
                     e.what());
    }
}

class TestConfigClass : public ConfigBase
{
public:
    TestConfigClass(const string& filename)
        : ConfigBase(filename)
    {
        const KnownEntries known_entries = {
                                              {  "SomeGroup",
                                                 {
                                                    "Empty",
                                                    "Zmq.Middleware",
                                                    "REST.Middleware",
                                                    "Zmq.BadMiddleware",
                                                    "REST.BadMiddleware"
                                                 }
                                              },
                                              {  "SomeOtherGroup",
                                                 {
                                                   "SomeString",
                                                   "SomeLocalizedString"
                                                 }
                                              }
                                           };
        check_unknown_entries(known_entries);
    }
};

TEST(ConfigBase, TestKnownEntries)
{
    {
        TestConfigClass c(TEST_INI);
        EXPECT_EQ("fred", c.parser()->get_locale_string("SomeOtherGroup", "SomeLocalizedString", "fr"));
    }
}
