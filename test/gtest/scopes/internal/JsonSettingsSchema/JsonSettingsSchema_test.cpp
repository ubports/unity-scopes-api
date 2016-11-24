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

#include <unity/scopes/internal/JsonSettingsSchema.h>

#include <unity/UnityExceptions.h>
#include <unity/util/FileIO.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity;
using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace std;

char const* ok_schema = R"delimiter(
{
    "settings":
    [
        {
            "id": "location",
            "displayName": "Location",
            "type": "string",
            "parameters": {
                "defaultValue": "London"
            }
        },
        {
            "id": "unitTemp",
            "displayName": "Temperature Units",
            "type": "list",
            "parameters": {
                "defaultValue": 1,
                "values": ["Celsius", "Fahrenheit"]
            }
        },
        {
            "id": "age",
            "displayName": "Age",
            "type": "number",
            "parameters": {
                "defaultValue": 23
            }
        },
        {
            "id": "enabled",
            "displayName": "Enabled",
            "type": "boolean",
            "parameters": {
                "defaultValue": true
            }
        },
        {
            "id": "string_no_default",
            "displayName": "string_no_default",
            "type": "string",
            "parameters": {
            }
        },
        {
            "id": "string_no_default2",
            "displayName": "string_no_default",
            "type": "string"
        },
        {
            "id": "list_no_default",
            "displayName": "list_no_default",
            "type": "list",
            "parameters": {
                "values": ["a", "b"]
            }
        },
        {
            "id": "number_no_default",
            "displayName": "number_no_default",
            "type": "number",
            "parameters": {
            }
        },
        {
            "id": "number_no_default2",
            "displayName": "number_no_default",
            "type": "number"
        },
        {
            "id": "boolean_no_default",
            "displayName": "boolean_no_default",
            "type": "boolean",
            "parameters": {
            }
        },
        {
            "id": "boolean_no_default2",
            "displayName": "boolean_no_default",
            "type": "boolean"
        }
    ]
}
)delimiter";

TEST(JsonSettingsSchema, basic)
{
    auto s = JsonSettingsSchema::create(ok_schema);

    auto defs = s->definitions();
    EXPECT_EQ(11u, defs.size());

    EXPECT_EQ("location", defs[0].get_dict()["id"].get_string());
    EXPECT_EQ("string", defs[0].get_dict()["type"].get_string());
    EXPECT_EQ("Location", defs[0].get_dict()["displayName"].get_string());
    EXPECT_EQ("London", defs[0].get_dict()["defaultValue"].get_string());

    EXPECT_EQ("unitTemp", defs[1].get_dict()["id"].get_string());
    EXPECT_EQ("list", defs[1].get_dict()["type"].get_string());
    EXPECT_EQ("Temperature Units", defs[1].get_dict()["displayName"].get_string());
    EXPECT_EQ(1, defs[1].get_dict()["defaultValue"].get_int());
    EXPECT_EQ(2u, defs[1].get_dict()["displayValues"].get_array().size());
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
    EXPECT_EQ(Variant(), defs[8].get_dict()["defaultValue"]);
    EXPECT_EQ(Variant(), defs[9].get_dict()["defaultValue"]);
    EXPECT_EQ(Variant(), defs[10].get_dict()["defaultValue"]);
}

TEST(JsonSettingsSchema, integer_sizes)
{
    char const* schema = R"delimiter(
    {
        "settings":
        [
            {
                "id": "double_temp",
                "displayName": "floating-point default",
                "type": "number",
                "parameters": {
                    "defaultValue": 3.14
                }
            },
            {
                "id": "too_old",
                "displayName": "won't fit into 32 bits",
                "type": "number",
                "parameters": {
                    "defaultValue": 2147483648
                }
            }
        ]
    }
    )delimiter";

    auto s = JsonSettingsSchema::create(schema);

    auto defs = s->definitions();
    EXPECT_EQ(2u, defs.size());

    EXPECT_EQ(3.14, defs[0].get_dict()["defaultValue"].get_double());
    EXPECT_EQ(2147483648, defs[1].get_dict()["defaultValue"].get_double());
}

TEST(JsonSettingsSchema, exceptions)
{
    try
    {
        JsonSettingsSchema::create("");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonCppNode(): empty string is not a valid JSON", e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"({ "a": "b" })");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): missing \"settings\" definition",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"({ "settings": "b" })");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): value \"settings\" must be an array",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "location",
                        "displayName": "Location",
                        "type": "string",
                        "parameters": {
                            "defaultValue": "London"
                        }
                    },
                    {
                        "id": "unitTemp",
                        "displayName": "Temperature Units",
                        "type": "list",
                        "parameters": {
                            "defaultValue": 1,
                            "values": ["Celsius", "Fahrenheit"]
                        }
                    },
                    {
                        "id": "location",
                        "displayName": "Age",
                        "type": "number",
                        "parameters": {
                            "defaultValue": 23
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): duplicate definition, id = \"location\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "",
                        "type": "number"
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid empty \"id\" definition",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "someid",
                        "displayName": "Location"
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): missing \"type\" definition, id = \"someid\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "someid": "id",
                        "displayName": "Location",
                        "type": 99
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): missing \"id\" definition",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "displayName": "Location",
                        "type": 99
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid value type for \"type\" definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "displayName": "Location",
                        "type": "no_such_type"
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid \"type\" definition: "
                     "\"no_such_type\", id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "string",
                        "parameters": 99
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): expected value of type object "
                     "for \"parameters\", id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "boolean",
                        "parameters": {
                            "defaultValue": "true"
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid value type "
                     "for \"defaultValue\" definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "number",
                        "displayName": "X",
                        "parameters": {
                            "defaultValue": "banana"
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid value type "
                     "for \"defaultValue\" definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list",
                        "parameters": {
                            "defaultValue": "hello",
                            "values": ["Celsius", "Fahrenheit"]
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid value type "
                     "for \"defaultValue\" definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list"
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): missing \"displayName\" definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list",
                        "parameters": {
                            "defaultValue": -1,
                            "values": "hello"
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid value type for \"values\" "
                     "definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list",
                        "parameters": {
                            "defaultValue": -1,
                            "values": [ ]
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid empty \"values\" definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list",
                        "parameters": {
                            "defaultValue": -1,
                            "values": [ 3, 7 ]
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid enumerator type, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list",
                        "parameters": {
                            "defaultValue": -1,
                            "values": [ "a", "" ]
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid empty enumerator, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list",
                        "parameters": {
                            "defaultValue": -1,
                            "values": [ "a", "b", "a" ]
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): duplicate enumerator \"a\", id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "list",
                        "parameters": {
                            "defaultValue": -1,
                            "values": [ "a", "b" ]
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): \"defaultValue\" out of range, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "x",
                        "type": "string",
                        "parameters": {
                            "defaultValue": true
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid value type "
                     "for \"defaultValue\" definition, id = \"x\"",
                     e.what());
    }

    try
    {
        JsonSettingsSchema::create(R"(
            {
                "settings":
                [
                    {
                        "id": "internal.x",
                        "type": "boolean",
                        "displayName": "Foo",
                        "parameters": {
                            "defaultValue": true
                        }
                    }
                ]
            }
        )");
        FAIL();
    }
    catch (ResourceException const& e)
    {
        EXPECT_STREQ("unity::ResourceException: JsonSettingsSchema(): invalid key \"internal.x\" prefixed with \"internal.\"",
                     e.what());
    }
}

TEST(JsonSettingsSchema, empty_then_with_location)
{
    auto s = JsonSettingsSchema::create_empty();
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
