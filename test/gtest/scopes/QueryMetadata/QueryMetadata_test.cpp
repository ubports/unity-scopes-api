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
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
    {
        SearchMetadata meta("pl", "phone");

        meta.set_internet_connectivity(QueryMetadata::Connected);
        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
        EXPECT_EQ(QueryMetadata::Connected, meta.internet_connectivity());
    }
    {
        SearchMetadata meta(50, "pl", "phone");

        EXPECT_EQ(50, meta.cardinality());
        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
    {
        SearchMetadata meta(50, "pl", "phone");

        meta.set_cardinality(100);
        meta["foo"] = "bar";
        meta.set_hint("baz", Variant(1000));
        EXPECT_EQ(100, meta.cardinality());
        EXPECT_EQ("bar", meta["foo"].get_string());
        EXPECT_EQ("bar", meta.hints()["foo"].get_string());
        EXPECT_EQ(1000, meta.hints()["baz"].get_int());
        EXPECT_TRUE(meta.contains_hint("foo"));
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
    {
        // referencing non-existing hint with a const object throws
        SearchMetadata const meta(50, "pl", "phone");
        EXPECT_THROW(meta["foo"], unity::LogicException);
        EXPECT_FALSE(meta.contains_hint("foo"));
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
}

TEST(SearchMetadata, serialize)
{
    {
        SearchMetadata meta("pl", "phone");
        meta["foo"] = "bar";

        auto var = meta.serialize();
        EXPECT_EQ("search_metadata", var["type"].get_string());
        EXPECT_EQ("pl", var["locale"].get_string());
        EXPECT_EQ("phone", var["form_factor"].get_string());
        EXPECT_EQ("bar", var["hints"].get_dict()["foo"].get_string());
        EXPECT_EQ(var.end(), var.find("internet_connectivity"));
    }
    {
        SearchMetadata meta("pl", "phone");
        meta["foo"] = "bar";
        meta.set_internet_connectivity(QueryMetadata::Disconnected);

        auto var = meta.serialize();
        EXPECT_EQ("search_metadata", var["type"].get_string());
        EXPECT_EQ("pl", var["locale"].get_string());
        EXPECT_EQ("phone", var["form_factor"].get_string());
        EXPECT_EQ("bar", var["hints"].get_dict()["foo"].get_string());
        EXPECT_FALSE(var["internet_connectivity"].get_bool());
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
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
    {
        SearchMetadata meta(100, "pl", "phone");
        SearchMetadata meta2(meta);

        meta.set_cardinality(0);
        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ(100, meta2.cardinality());
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
    {
        SearchMetadata meta(100, "pl", "phone");
        auto meta2 = meta;

        meta.set_cardinality(0);
        meta.set_internet_connectivity(QueryMetadata::Connected);
        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ(100, meta2.cardinality());
        EXPECT_EQ(QueryMetadata::Connected, meta.internet_connectivity());
    }
    {
        SearchMetadata meta(100, "pl", "phone");
        SearchMetadata meta2(meta);

        meta.set_cardinality(0);
        meta.set_internet_connectivity(QueryMetadata::Disconnected);
        EXPECT_EQ(0, meta.cardinality());
        EXPECT_EQ(100, meta2.cardinality());
        EXPECT_EQ(QueryMetadata::Disconnected, meta.internet_connectivity());
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
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
    {
        VariantMap var;
        var["foo"] = "bar";
        ActionMetadata meta("pl", "phone");
        meta.set_scope_data(Variant(var));
        meta.set_internet_connectivity(QueryMetadata::Disconnected);

        EXPECT_EQ("pl", meta.locale());
        EXPECT_EQ("phone", meta.form_factor());
        EXPECT_EQ("bar", meta.scope_data().get_dict()["foo"].get_string());
        EXPECT_EQ(QueryMetadata::Disconnected, meta.internet_connectivity());
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
        EXPECT_EQ(var.end(), var.find("internet_connectivity"));

        // deserialize
        auto meta2 = internal::ActionMetadataImpl::create(var);
        EXPECT_EQ("pl", meta2.locale());
        EXPECT_EQ("phone", meta2.form_factor());
        EXPECT_EQ(1234, meta2.scope_data().get_int());
        EXPECT_EQ(QueryMetadata::Unknown, meta2.internet_connectivity());
    }

    {
        ActionMetadata meta("pl", "phone");
        meta.set_scope_data(Variant(1234));
        meta.set_internet_connectivity(QueryMetadata::Connected);

        auto var = meta.serialize();
        EXPECT_EQ("action_metadata", var["type"].get_string());
        EXPECT_EQ("pl", var["locale"].get_string());
        EXPECT_EQ("phone", var["form_factor"].get_string());
        EXPECT_EQ(1234, var["scope_data"].get_int());
        EXPECT_TRUE(var["internet_connectivity"].get_bool());

        // deserialize
        auto meta2 = internal::ActionMetadataImpl::create(var);
        EXPECT_EQ("pl", meta2.locale());
        EXPECT_EQ("phone", meta2.form_factor());
        EXPECT_EQ(1234, meta2.scope_data().get_int());
        EXPECT_EQ(QueryMetadata::Connected, meta2.internet_connectivity());
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
        EXPECT_EQ(QueryMetadata::Unknown, meta.internet_connectivity());
    }
    {
        ActionMetadata meta("pl", "phone");
        ActionMetadata meta2(meta);

        Variant var(10);
        meta.set_scope_data(var);
        meta.set_internet_connectivity(QueryMetadata::Disconnected);

        EXPECT_TRUE(meta2.scope_data().is_null());
        EXPECT_EQ(10, meta.scope_data().get_int());
        EXPECT_EQ(QueryMetadata::Disconnected, meta.internet_connectivity());
    }
}
