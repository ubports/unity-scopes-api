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

#include <unity/scopes/VariantBuilder.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity;
using namespace unity::scopes;

TEST(VariantBuilder, basic)
{
    VariantBuilder builder;
    {
        // create an array of tuples:
        //   [{"a" : 1, b : 2}, {"c" : null, "d" : "xyz"}]
        builder.add_tuple({{"a", Variant(1)}, {"b", Variant(2)}});
        builder.add_tuple({{"c", Variant::null()}, {"d", Variant("xyz")}});

        auto arr = builder.end().get_array();
        EXPECT_EQ(2u, arr.size());
        EXPECT_EQ(1, arr[0].get_dict()["a"].get_int());
        EXPECT_EQ(2, arr[0].get_dict()["b"].get_int());

        EXPECT_TRUE(arr[1].get_dict()["c"].is_null());
        EXPECT_EQ("xyz", arr[1].get_dict()["d"].get_string());
    }
    // reusing a builder
    {
        EXPECT_THROW(builder.end(), unity::LogicException);
        builder.add_tuple({{"a", Variant(0)}});
        EXPECT_EQ(1u, builder.end().get_array().size());
    }
}

TEST(VariantBuilder, copy)
{
    {
        VariantBuilder builder;
        builder.add_tuple({{"a", Variant(0)}});
        VariantBuilder builder2 = builder;
        EXPECT_EQ(1u, builder.end().get_array().size());
        EXPECT_EQ(1u, builder2.end().get_array().size());
    }
}

namespace unity
{

namespace scopes
{

} // namespace scopes

} // namespace unity
