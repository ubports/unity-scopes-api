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

#include <unity/scopes/VariantMapBuilder.h>
#include <gtest/gtest.h>

using namespace unity;
using namespace unity::scopes;

TEST(VariantMapBuilder, basic)
{
    VariantMapBuilder builder;
    // {
    //    "foo" : "bar",
    //    "mykey" : [{"a" : 1, b : 2}, {"c" : null, "d" : "xyz"}]
    // }
    builder.add_attribute("foo", Variant("bar"));
    builder.add_tuple("mykey", {{"a", Variant(1)}, {"b", Variant(2)}});
    builder.add_tuple("mykey", {{"c", Variant::null()}, {"d", Variant("xyz")}});

    auto var = builder.variant_map();
    EXPECT_EQ(2, var.size());
    EXPECT_EQ("bar", var["foo"].get_string());
    auto arr = var["mykey"].get_array();
    EXPECT_EQ(2, arr.size());
    EXPECT_EQ(1, arr[0].get_dict()["a"].get_int());
    EXPECT_EQ(2, arr[0].get_dict()["b"].get_int());

    EXPECT_TRUE(arr[1].get_dict()["c"].is_null());
    EXPECT_EQ("xyz", arr[1].get_dict()["d"].get_string());
}

namespace unity
{

namespace scopes
{

} // namespace scopes

} // namespace unity
