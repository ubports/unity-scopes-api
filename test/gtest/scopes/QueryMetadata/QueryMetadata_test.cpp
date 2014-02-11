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
#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/internal/ActionMetadataImpl.h>
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

TEST(ActionMetadata, basic)
{
    {
        VariantMap var;
        var["foo"] = "bar";
        ActionMetadata meta("pl", "phone");
        meta.set_scope_data(Variant(var));

        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
        EXPECT_EQ("bar", meta.scope_data().get_dict()["foo"].get_string());
    }
}

TEST(ActionMetadata, serialize_and_deserialize)
{
    {
        ActionMetadata meta("pl", "phone");
        meta.set_scope_data(Variant(1234));

        auto var = meta.serialize();
        EXPECT_EQ("action_metadata", var["type"].get_string());
        EXPECT_EQ("pl", var["locale"].get_string());
        EXPECT_EQ("phone", var["form_factor"].get_string());
        EXPECT_EQ(1234, var["scope_data"].get_int());

        // deserialize
        auto meta2 = internal::ActionMetadataImpl::create(var);
        EXPECT_EQ("pl", meta2.locale());
        EXPECT_EQ("phone", meta2.form_factor());
        EXPECT_EQ(1234, meta2.scope_data().get_int());
    }
}

TEST(ActionMetadata, copy)
{
    {
        ActionMetadata meta("pl", "phone");
        auto meta2 = meta;

        Variant var("foo");
        meta.set_scope_data(var);

        EXPECT_TRUE(meta2.scope_data().is_null());
        EXPECT_EQ("foo", meta.scope_data().get_string());
    }
    {
        ActionMetadata meta("pl", "phone");
        ActionMetadata meta2(meta);

        Variant var(10);
        meta.set_scope_data(var);

        EXPECT_TRUE(meta2.scope_data().is_null());
        EXPECT_EQ(10, meta.scope_data().get_int());
    }
}
