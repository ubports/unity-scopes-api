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
        SearchMetadata meta("pl", "phone");

        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
    }
    {
        SearchMetadata meta(50, "pl", "phone");

        EXPECT_EQ(50, meta.cardinality());
        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
    }
    {
        SearchMetadata meta(50, "pl", "phone");

        meta.set_cardinality(100);
        EXPECT_EQ(100, meta.cardinality());
    }
}

TEST(SearchMetadata, serialize)
{
    {
        SearchMetadata meta("pl", "phone");

        auto var = meta.serialize();
        EXPECT_EQ("search_metadata", var["type"].get_string());
        EXPECT_EQ("pl", var["locale"].get_string());
        EXPECT_EQ("phone", var["form_factor"].get_string());
    }
}

TEST(SearchMetadata, copy)
{
    {
        SearchMetadata meta(100, "pl", "phone");
        auto meta2 = meta;

        meta.set_cardinality(0);
        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ(100, meta2.cardinality());
    }
    {
        SearchMetadata meta(100, "pl", "phone");
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
