/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/internal/IniSettingsSchema.h>

#include <unity/UnityExceptions.h>

#include <boost/regex.hpp>  // Use Boost implementation until http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53631 is fixed.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace std;

TEST(IniSettingsSchema, basic)
{
    auto s = IniSettingsSchema::create(TEST_SRC_DIR "/schema.ini");
    s->add_location_setting();

    auto defs = s->definitions();
    ASSERT_EQ(9u, defs.size());

    EXPECT_EQ("location", defs[0].get_dict()["id"].get_string());
    EXPECT_EQ("string", defs[0].get_dict()["type"].get_string());
    EXPECT_EQ("Location", defs[0].get_dict()["displayName"].get_string());
    EXPECT_EQ("London", defs[0].get_dict()["defaultValue"].get_string());

    EXPECT_EQ("tempUnit", defs[1].get_dict()["id"].get_string());
    EXPECT_EQ("list", defs[1].get_dict()["type"].get_string());
    EXPECT_EQ("Temperature Unit", defs[1].get_dict()["displayName"].get_string());
    EXPECT_EQ(1, defs[1].get_dict()["defaultValue"].get_int());
    ASSERT_EQ(2u, defs[1].get_dict()["displayValues"].get_array().size());
    EXPECT_EQ("Celsius", defs[1].get_dict()["displayValues"].get_array()[0].get_string());
    EXPECT_EQ("Fahrenheit", defs[1].get_dict()["displayValues"].get_array()[1].get_string());

    EXPECT_EQ("age", defs[2].get_dict()["id"].get_string());
    EXPECT_EQ("number", defs[2].get_dict()["type"].get_string());
    EXPECT_EQ("Age", defs[2].get_dict()["displayName"].get_string());
    EXPECT_EQ(23, defs[2].get_dict()["defaultValue"].get_double());

    EXPECT_EQ("enabled", defs[3].get_dict()["id"].get_string());
    EXPECT_EQ("boolean", defs[3].get_dict()["type"].get_string());
    EXPECT_EQ("Enabled", defs[3].get_dict()["displayName"].get_string());
    EXPECT_TRUE(defs[3].get_dict()["defaultValue"].get_bool());

    EXPECT_EQ(Variant(), defs[4].get_dict()["defaultValue"]);

    EXPECT_EQ(Variant(), defs[5].get_dict()["defaultValue"]);
    EXPECT_EQ(Variant(), defs[6].get_dict()["defaultValue"]);
    EXPECT_EQ(Variant(), defs[7].get_dict()["defaultValue"]);
}

TEST(IniSettingsSchema, exceptions)
{
    try
    {
        IniSettingsSchema::create("no_such_file");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: IniSettingsSchema(): cannot parse settings file \"no_such_file\":\n"
                     "    unity::FileException: Could not load ini file no_such_file: No such file or directory (errno = 4)",
                     e.what());
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/missing_type.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n"
                       "    unity::ResourceException: IniSettingsSchema\\(\\): missing \"type\" definition, setting = \"location\":\n"
                       "        unity::LogicException:.*");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/bad_type.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n"
                       "    unity::ResourceException: IniSettingsSchema\\(\\): invalid \"type\" definition: \"99\", "
                       "setting = \"location\"");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/bad_list.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n"
                       "    unity::ResourceException: IniSettingsSchema\\(\\): invalid number of entries for \"displayValues\" "
                       "definition, setting = \"tempUnit\"");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/bad_list_default.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n"
                       "    unity::ResourceException: IniSettingsSchema\\(\\): \"displayValues\" out of range, setting = \"tempUnit\"");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/missing_list_values.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n"
                       "    unity::ResourceException: IniSettingsSchema\\(\\): missing \"displayValues\" definition, "
                       "setting = \"tempUnit\":.*");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/bad_bool.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n.*");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/missing_display_name.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n"
                       "    unity::ResourceException: IniSettingsSchema\\(\\): missing \"displayName\" definition, setting = \"location\":\n"
                       "        unity::LogicException:.*");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }

    try
    {
        IniSettingsSchema::create(TEST_SRC_DIR "/internal_name.ini");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        boost::regex r("unity::ResourceException: IniSettingsSchema\\(\\): cannot parse settings file \".*\":\n"
                       "    unity::ResourceException: IniSettingsSchema\\(\\): invalid key \"internal.foo\" prefixed with \"internal.\"");
        EXPECT_TRUE(boost::regex_match(e.what(), r)) << e.what();
    }
}

class SetLanguage: public ::testing::Test
{
public:
    SetLanguage()
    {
        setenv("LANGUAGE", "test", 1);
    }

    ~SetLanguage()
    {
        unsetenv("LANGUAGE");
    }
};

TEST_F(SetLanguage, localization)
{
    {
        auto s = IniSettingsSchema::create(TEST_SRC_DIR "/schema.ini");

        auto defs = s->definitions();

        EXPECT_EQ("tempUnit", defs[1].get_dict()["id"].get_string());
        EXPECT_EQ("testTemperature Unit", defs[1].get_dict()["displayName"].get_string());
        ASSERT_EQ(2u, defs[1].get_dict()["displayValues"].get_array().size());
        EXPECT_EQ("testCelsius", defs[1].get_dict()["displayValues"].get_array()[0].get_string());
        EXPECT_EQ("testFahrenheit", defs[1].get_dict()["displayValues"].get_array()[1].get_string());
    }

    // Check that, if the locale is set, but no strings are defined for that locale,
    // the non-localized version is returned.
    {
        auto s = IniSettingsSchema::create(TEST_SRC_DIR "/locale_fallback.ini");

        auto defs = s->definitions();
        ASSERT_EQ(1u, defs.size());

        EXPECT_EQ("tempUnit", defs[0].get_dict()["id"].get_string());
        EXPECT_EQ("Temperature Unit", defs[0].get_dict()["displayName"].get_string());
        ASSERT_EQ(2u, defs[0].get_dict()["displayValues"].get_array().size());
        EXPECT_EQ("Celsius", defs[0].get_dict()["displayValues"].get_array()[0].get_string());
        EXPECT_EQ("Fahrenheit", defs[0].get_dict()["displayValues"].get_array()[1].get_string());
    }
}

TEST(IniSettingsSchema, empty_then_with_location)
{
    auto s = IniSettingsSchema::create_empty();
    {
        auto defs = s->definitions();
        ASSERT_TRUE(defs.empty());
    }
    s->add_location_setting();
    {
        auto defs = s->definitions();
        ASSERT_EQ(1u, defs.size());

        EXPECT_EQ("internal.location", defs[0].get_dict()["id"].get_string());
        EXPECT_EQ("Enable location data", defs[0].get_dict()["displayName"].get_string());
        EXPECT_EQ("boolean", defs[0].get_dict()["type"].get_string());
        EXPECT_TRUE(defs[0].get_dict()["displayValues"].is_null());
        EXPECT_TRUE(defs[0].get_dict()["defaultValue"].get_bool());
    }
}
