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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
*/

#include <unity/scopes/internal/JsonUtils.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(json_to_variant, basic)
{
    auto var = json_to_variant(R"({"a":1, "b": 2.0, "c":null, "d":[1,true,"foo"]})");
    auto outer = var.get_dict();
    EXPECT_EQ(4, outer.size());
    EXPECT_EQ(1, outer["a"].get_int());
    EXPECT_TRUE(outer["b"].get_double() - 2.0f < 0.00001f);
    EXPECT_TRUE(outer["c"].is_null());
    auto arr = outer["d"].get_array();
    EXPECT_EQ(3, arr.size());
    EXPECT_EQ(1, arr[0].get_int());
    EXPECT_TRUE(arr[1].get_bool());
    EXPECT_EQ("foo", arr[2].get_string());
}
