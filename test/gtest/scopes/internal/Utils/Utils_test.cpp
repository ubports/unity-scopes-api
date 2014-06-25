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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/Utils.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Utils, uncamelcase)
{
    EXPECT_EQ("", uncamelcase(""));
    EXPECT_EQ("foo-bar", uncamelcase("FooBar"));
    EXPECT_EQ("foo-bar", uncamelcase("foo-bar"));
    EXPECT_EQ("foo_bar", uncamelcase("foo_bar"));
    EXPECT_EQ("foo-bar", uncamelcase("fooBAR"));
    EXPECT_EQ("foo-bar", uncamelcase("fooBAr"));
    EXPECT_EQ("foo-bar", uncamelcase("foo-Bar"));
}

TEST(Utils, convert_to)
{
    {
        Variant out;
        EXPECT_FALSE(convert_to<int>("foo", out));
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<int>("10", out));
        EXPECT_EQ(10, out.get_int());
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<std::string>("foo", out));
        EXPECT_EQ("foo", out.get_string());
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<double>("10.0", out));
        EXPECT_TRUE(out.get_double() - 10.0f < 0.0001f);
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<bool>("true", out));
        EXPECT_TRUE(out.get_bool());
    }
    {
        Variant out;
        EXPECT_TRUE(convert_to<bool>("false", out));
        EXPECT_FALSE(out.get_bool());
    }
}
