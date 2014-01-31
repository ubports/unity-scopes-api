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

#include <unity/scopes/SearchMetadata.h>
#include <gtest/gtest.h>
#include <unity/UnityExceptions.h>

using namespace unity;
using namespace unity::scopes;

TEST(SearchMetadata, basic)
{
    {
        GeoCoordinate coord(10.0f, 20.0f, 100.0f);
        SearchMetadata meta("pl", "phone", coord);

        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
        EXPECT_TRUE(abs(meta.location().latitude() - 10.0f) <= 0.0000001f);
        EXPECT_TRUE(abs(meta.location().longitude() - 20.0f) <= 0.0000001f);
        EXPECT_TRUE(meta.location().has_altitude());
        EXPECT_TRUE(abs(meta.location().altitude() - 100.f) <= 0.0000001f);
    }
    {
        GeoCoordinate coord(10.0f, 20.0f);
        SearchMetadata meta(50, "pl", "phone", coord);

        EXPECT_EQ(50, meta.cardinality());
        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
        EXPECT_TRUE(abs(meta.location().latitude() - 10.0f) <= 0.0000001f);
        EXPECT_TRUE(abs(meta.location().longitude() - 20.0f) <= 0.0000001f);
        EXPECT_FALSE(meta.location().has_altitude());
        EXPECT_TRUE(abs(meta.location().altitude()) <= 0.0000001f);
    }
    {
        GeoCoordinate coord(10.0f, 20.0f);
        SearchMetadata meta(50, "pl", "phone", coord);

        meta.set_cardinality(100);
        EXPECT_EQ(100, meta.cardinality());
    }
}

TEST(SearchMetadata, serialize)
{
    {
        GeoCoordinate coord(10.0f, 20.0f, 100.0f);
        SearchMetadata meta("pl", "phone", coord);

        auto var = meta.serialize();
        EXPECT_EQ("search_metadata", var["type"].get_string());
        EXPECT_EQ("pl", var["locale"].get_string());
        EXPECT_EQ("phone", var["form_factor"].get_string());
        EXPECT_EQ(true, var["location"].get_dict()["has_alt"].get_bool());
        EXPECT_TRUE(abs(var["location"].get_dict()["lat"].get_double() - 10.0f) <= 0.000001f);
        EXPECT_TRUE(abs(var["location"].get_dict()["lon"].get_double() - 20.0f) <= 0.000001f);
        EXPECT_TRUE(abs(var["location"].get_dict()["alt"].get_double() - 100.0f) <= 0.000001f);
    }
}

TEST(SearchMetadata, copy)
{
    {
        GeoCoordinate coord(10.0f, 20.0f, 100.0f);
        SearchMetadata meta(100, "pl", "phone", coord);
        auto meta2 = meta;

        meta.set_cardinality(0);
        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ(100, meta2.cardinality());
    }
    {
        GeoCoordinate coord(10.0f, 20.0f, 100.0f);
        SearchMetadata meta(100, "pl", "phone", coord);
        SearchMetadata meta2(meta);
        meta.set_cardinality(0);

        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ(100, meta2.cardinality());
    }
}

namespace unity
{

namespace scopes
{

} // namespace scopes

} // namespace unity
