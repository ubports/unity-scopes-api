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

#include <unity/scopes/Variant.h>
#include <unity/UnityExceptions.h>

#include <boost/variant.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace std;
using namespace unity;
using namespace unity::scopes;

TEST(Variant, basic)
{
    {
        Variant v;
        EXPECT_EQ(Variant::Type::Null, v.which());
        // No exception here means that v was initialized to an int with unknown value.
    }

    {
        Variant v(42);
        EXPECT_EQ(Variant::Type::Int, v.which());
        EXPECT_EQ(42, v.get_int());
    }

    {
        int64_t x = 99;
        Variant v(x);
        EXPECT_EQ(Variant::Type::Int64, v.which());
        EXPECT_EQ(99, v.get_int64_t());
    }

    {
        Variant v(10.5f);
        EXPECT_EQ(Variant::Type::Double, v.which());
        EXPECT_TRUE(v.get_double() - 10.5f < 0.00001f);
    }

    {
        Variant v(true);
        EXPECT_EQ(Variant::Type::Bool, v.which());
        EXPECT_TRUE(v.get_bool());
    }

    {
        Variant v(string("hello"));
        EXPECT_EQ(Variant::Type::String, v.which());
        EXPECT_EQ("hello", v.get_string());
    }

    {
        Variant v("hello");
        EXPECT_EQ(Variant::Type::String, v.which());
        EXPECT_EQ("hello", v.get_string());
    }

    {
        VariantMap m;
        VariantMap inner;
        inner["iron"] = Variant("maiden");
        m["foo"] = Variant("bar");
        m["hints"] = inner;
        Variant v(m);
        EXPECT_EQ(Variant::Type::Dict, v.which());
        EXPECT_EQ("bar", v.get_dict()["foo"].get_string());
        EXPECT_EQ(Variant::Type::Dict, v.get_dict()["hints"].which());
        EXPECT_EQ("maiden", v.get_dict()["hints"].get_dict()["iron"].get_string());
    }

    {
        VariantArray varr {Variant(1), Variant("foo")};
        Variant v(varr);
        EXPECT_EQ(Variant::Type::Array, v.which());
        EXPECT_EQ(2u, v.get_array().size());
        EXPECT_EQ(1, v.get_array()[0].get_int());
        EXPECT_EQ("foo", v.get_array()[1].get_string());
    }

    {
        // Copy constructor
        Variant v(42);
        Variant v2(v);
        EXPECT_EQ(42, v2.get_int());
    }

    {
        // Assignment operators

        Variant v("hello");
        Variant v2(42);

        v = v2;
        EXPECT_EQ(42, v.get_int());

        v = 99;
        EXPECT_EQ(99, v.get_int());

        v = 10.5f;
        EXPECT_TRUE(v.get_double() - 10.5f < 0.00001f);

        v = false;
        EXPECT_FALSE(v.get_bool());

        v = "hello";
        EXPECT_EQ("hello", v.get_string());

        v = string("world");
        EXPECT_EQ("world", v.get_string());
    }

    {
        // Comparison operators

        Variant v1(1);
        Variant v2(2);
        EXPECT_TRUE(v1 < v2);
        EXPECT_FALSE(v1 == v2);

        v2 = "hello";
        EXPECT_TRUE(v1 < v2);   // Any int is less than any string
        EXPECT_FALSE(v2 < v1);
        EXPECT_TRUE(v1 < v2);
        EXPECT_FALSE(v1 == v2);

        v1 = v2;
        EXPECT_FALSE(v1 < v2);
        EXPECT_FALSE(v2 < v1);
        EXPECT_TRUE(v1 == v2);

        v1 = 5;
        v2 = false;
        EXPECT_TRUE(v1 < v2);   // Any int is less than any bool

        v1 = true;
        v2 = "";
        EXPECT_TRUE(v1 < v2);   // Any bool is less than any string
    }

    {
        // swap

        Variant v1(1);
        Variant v2(2);
        swap(v1, v2);
        EXPECT_EQ(2, v1.get_int());
        EXPECT_EQ(1, v2.get_int());

        v2 = "hello";
        swap(v1, v2);
        EXPECT_EQ("hello", v1.get_string());
        EXPECT_EQ(2, v2.get_int());
    }
}

TEST(Variant, null)
{
    Variant v0(0);
    EXPECT_FALSE(v0.is_null());

    Variant v1 = Variant::null();
    EXPECT_TRUE(v1.is_null());

    Variant v2 = v1;
    EXPECT_TRUE(v2.is_null());

    Variant v3(v1);
    EXPECT_TRUE(v3.is_null());

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 < v2);
    EXPECT_FALSE(v2 < v1);
}

TEST(Variant, exceptions)
{
    try
    {
        Variant v;
        v.get_bool();
        FAIL();
    }
    catch (LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: Variant does not contain a bool value:\n"
                     "    boost::bad_get: failed value get using boost::get",
                     e.what());
    }

    try
    {
        Variant v;
        v.get_string();
        FAIL();
    }
    catch (LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: Variant does not contain a string value:\n"
                     "    boost::bad_get: failed value get using boost::get",
                     e.what());
    }

    try
    {
        Variant v("hello");
        v.get_int();
        FAIL();
    }
    catch (LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: Variant does not contain an int value:\n"
                     "    boost::bad_get: failed value get using boost::get",
                     e.what());
    }

}

TEST(Variant, serialize_json)
{
    {
        Variant v;
        EXPECT_EQ("null\n", v.serialize_json());
    }
    {
        Variant v(42);
        EXPECT_EQ("42\n", v.serialize_json());
    }

    {
        Variant v(10.5f);
        auto const str = v.serialize_json();
        EXPECT_TRUE(str == "10.50\n" || str == "10.5\n");
    }

    {
        Variant v(true);
        EXPECT_EQ("true\n", v.serialize_json());
    }

    {
        Variant v("hello");
        EXPECT_EQ("\"hello\"\n", v.serialize_json());
    }

    {
        VariantMap m;
        m["foo"] = Variant("bar");
        Variant v(m);
        EXPECT_EQ("{\"foo\":\"bar\"}\n", v.serialize_json());
    }

    {
        VariantArray varr {Variant(1), Variant("foo")};
        Variant v(varr);
        EXPECT_EQ("[1,\"foo\"]\n", v.serialize_json());
    }
}

TEST(Variant, deserialize_json)
{
    Variant v = Variant::deserialize_json("null");
    EXPECT_TRUE(v.is_null());

    v = Variant::deserialize_json("42");
    EXPECT_EQ(42, v.get_int());

    v = Variant::deserialize_json("10.5");
    EXPECT_EQ(10.5f, v.get_double());

    v = Variant::deserialize_json("false");
    EXPECT_FALSE(v.get_bool());

    v = Variant::deserialize_json("\"hello\"");
    EXPECT_EQ("hello", v.get_string());

    v = Variant::deserialize_json("{\"foo\": \"bar\"}");
    VariantMap vm = v.get_dict();
    EXPECT_EQ(1u, vm.size());
    EXPECT_EQ("bar", vm.at("foo").get_string());

    v = Variant::deserialize_json("[1, \"two\"]");
    VariantArray va = v.get_array();
    EXPECT_EQ(2u, va.size());
    EXPECT_EQ(1, va[0].get_int());
    EXPECT_EQ("two", va[1].get_string());
}
