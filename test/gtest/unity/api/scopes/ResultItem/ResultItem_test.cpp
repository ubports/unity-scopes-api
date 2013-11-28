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

#include <scopes/ResultItem.h>
#include <scopes/Category.h>
#include <scopes/CategoryRenderer.h>
#include <scopes/internal/CategoryRegistry.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

// basic test of ResultIem setters and getters
TEST(ResultItem, basic)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    auto cat2 = reg.register_category("2", "title", "icon", rdr);

    {
        ResultItem result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_title("a title");
        result.set_art("an icon");
        result.set_dnd_uri("http://canonical.com");
        result.add_metadata("foo", Variant("bar"));

        EXPECT_EQ("http://ubuntu.com", result.uri());
        EXPECT_EQ("a title", result.title());
        EXPECT_EQ("an icon", result.art());
        EXPECT_EQ("http://canonical.com", result.dnd_uri());
        EXPECT_EQ("bar", result.serialize()["foo"].get_string());
        EXPECT_EQ("1", result.category()->id());

        result.set_category(cat2);
        EXPECT_EQ("2", result.category()->id());
    }

    // copy ctor
    {
        ResultItem result(cat);
        result.set_uri("uri a");
        result.set_title("title a");
        result.set_art("icon a");
        result.set_dnd_uri("dnd_uri a");

        ResultItem copy(result);

        copy.set_uri("uri b");
        copy.set_title("title b");
        copy.set_art("icon b");
        copy.set_dnd_uri("dnd_uri b");

        result.add_metadata("foo", Variant("bar"));
        copy.add_metadata("foo", Variant("xyz"));

        EXPECT_EQ("uri a", result.uri());
        EXPECT_EQ("title a", result.title());
        EXPECT_EQ("icon a", result.art());
        EXPECT_EQ("dnd_uri a", result.dnd_uri());
        EXPECT_EQ("bar", result.serialize()["foo"].get_string());
        EXPECT_EQ("1", result.category()->id());

        EXPECT_EQ("uri b", copy.uri());
        EXPECT_EQ("title b", copy.title());
        EXPECT_EQ("icon b", copy.art());
        EXPECT_EQ("dnd_uri b", copy.dnd_uri());
        EXPECT_EQ("xyz", copy.serialize()["foo"].get_string());
        EXPECT_EQ("1", copy.category()->id());
    }

    // assignment copy
    {
        ResultItem result(cat);
        result.set_uri("uri a");
        result.set_title("title a");
        result.set_art("icon a");
        result.set_dnd_uri("dnd_uri a");
        ResultItem copy = result;

        copy.set_uri("uri b");
        copy.set_title("title b");
        copy.set_art("icon b");
        copy.set_dnd_uri("dnd_uri b");

        result.add_metadata("foo", Variant("bar"));
        copy.add_metadata("foo", Variant("xyz"));

        EXPECT_EQ("uri a", result.uri());
        EXPECT_EQ("title a", result.title());
        EXPECT_EQ("icon a", result.art());
        EXPECT_EQ("dnd_uri a", result.dnd_uri());
        EXPECT_EQ("bar", result.serialize()["foo"].get_string());
        EXPECT_EQ("1", result.category()->id());

        EXPECT_EQ("uri b", copy.uri());
        EXPECT_EQ("title b", copy.title());
        EXPECT_EQ("icon b", copy.art());
        EXPECT_EQ("dnd_uri b", copy.dnd_uri());
        EXPECT_EQ("xyz", copy.serialize()["foo"].get_string());
        EXPECT_EQ("1", copy.category()->id());

        ResultItem copy2(cat);
        copy2 = copy;
        EXPECT_EQ("uri b", copy2.uri());
        EXPECT_EQ("title b", copy2.title());
        EXPECT_EQ("icon b", copy2.art());
        EXPECT_EQ("dnd_uri b", copy2.dnd_uri());
        EXPECT_EQ("xyz", copy2.serialize()["foo"].get_string());
        EXPECT_EQ("1", copy2.category()->id());
    }
}

// test conversion to VariantMap
TEST(ResultItem, serialize)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    ResultItem result(cat);
    result.set_uri("http://ubuntu.com");
    result.set_title("a title");
    result.set_art("an icon");
    result.set_dnd_uri("http://canonical.com");

    EXPECT_EQ("http://ubuntu.com", result.uri());
    EXPECT_EQ("a title", result.title());
    EXPECT_EQ("an icon", result.art());
    EXPECT_EQ("http://canonical.com", result.dnd_uri());

    auto var = result.serialize();
    EXPECT_EQ("http://ubuntu.com", var["uri"].get_string());
    EXPECT_EQ("a title", var["title"].get_string());
    EXPECT_EQ("an icon", var["art"].get_string());
    EXPECT_EQ("http://canonical.com", var["dnd_uri"].get_string());
    EXPECT_EQ("1", var["cat_id"].get_string());
}

// test exceptions when converting to VariantMap
TEST(ResultItem, serialize_excp)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    ResultItem result(cat);

    // throw until all required attributes are non-empty
    EXPECT_THROW(result.serialize(), unity::InvalidArgumentException);
    result.set_uri("http://ubuntu.com");
    EXPECT_THROW(result.serialize(), unity::InvalidArgumentException);
    result.set_title("a title");
    EXPECT_THROW(result.serialize(), unity::InvalidArgumentException);
    result.set_art("an icon");
    EXPECT_THROW(result.serialize(), unity::InvalidArgumentException);
    result.set_dnd_uri("http://canonical.com");
    EXPECT_NO_THROW(result.serialize());
}

// test exceptions with null category
TEST(ResultItem, exceptions)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    Category::SCPtr null_cat;

    bool excp = false;
    try
    {
        ResultItem r(null_cat);
    }
    catch (const unity::InvalidArgumentException& e)
    {
        excp = true;
    }
    EXPECT_TRUE(excp);

    ResultItem result(cat);
    EXPECT_THROW(result.set_category(null_cat), unity::InvalidArgumentException);
}

// test conversion from VariantMap
TEST(ResultItem, deserialize)
{
    VariantMap vm;
    vm["uri"] = "http://ubuntu.com";
    vm["dnd_uri"] = "http://canonical.com";
    vm["title"] = "a title";
    vm["art"] = "an icon";
    vm["cat_id"] = "2";
    vm["foo"] = "bar"; // custom attribute

    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    ResultItem result(cat, vm);

    auto var = result.serialize();
    EXPECT_EQ("http://ubuntu.com", var["uri"].get_string());
    EXPECT_EQ("a title", var["title"].get_string());
    EXPECT_EQ("an icon", var["art"].get_string());
    EXPECT_EQ("http://canonical.com", var["dnd_uri"].get_string());
    EXPECT_EQ("bar", var["foo"].get_string());
    EXPECT_EQ("1", var["cat_id"].get_string());
}
