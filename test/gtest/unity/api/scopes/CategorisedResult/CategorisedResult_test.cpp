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

#include <scopes/CategorisedResult.h>
#include <scopes/Category.h>
#include <scopes/CategoryRenderer.h>
#include <scopes/internal/CategoryRegistry.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

// basic test of CategorisedResultIem setters and getters
TEST(CategorisedResult, basic)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    auto cat2 = reg.register_category("2", "title", "icon", rdr);

    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_title("a title");
        result.set_art("an icon");
        result.set_dnd_uri("http://canonical.com");
        result.add_metadata("foo", Variant("bar"));

        EXPECT_EQ("http://ubuntu.com", result.uri());
        EXPECT_EQ("a title", result.title());
        EXPECT_EQ("an icon", result.art());
        EXPECT_EQ("http://canonical.com", result.dnd_uri());
        EXPECT_EQ("bar", result.serialize()["attrs"].get_dict()["foo"].get_string());
        EXPECT_EQ("1", result.category()->id());
    }
}

TEST(CategorisedResult, copy)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    // copy ctor
    {
        CategorisedResult result(cat);
        result.set_uri("uri a");
        result.set_title("title a");
        result.set_art("icon a");
        result.set_dnd_uri("dnd_uri a");
        result.add_metadata("common", Variant("o"));

        CategorisedResult copy(result);

        copy.set_uri("uri b");
        copy.set_title("title b");
        copy.set_art("icon b");
        copy.set_dnd_uri("dnd_uri b");

        result.add_metadata("foo", Variant("bar"));
        result.add_metadata("unique", Variant(123));
        copy.add_metadata("foo", Variant("xyz"));

        {
            auto attrsvar = result.serialize()["attrs"].get_dict();
            EXPECT_EQ("uri a", result.uri());
            EXPECT_EQ("title a", result.title());
            EXPECT_EQ("icon a", result.art());
            EXPECT_EQ("dnd_uri a", result.dnd_uri());
            EXPECT_EQ("o", attrsvar["common"].get_string());
            EXPECT_EQ(123, attrsvar["unique"].get_int());
            EXPECT_EQ("bar", attrsvar["foo"].get_string());
            EXPECT_EQ("1", result.category()->id());
        }
        {
            auto attrsvar = copy.serialize()["attrs"].get_dict();
            EXPECT_EQ("uri b", copy.uri());
            EXPECT_EQ("title b", copy.title());
            EXPECT_EQ("icon b", copy.art());
            EXPECT_EQ("dnd_uri b", copy.dnd_uri());
            EXPECT_EQ("o", attrsvar["common"].get_string());
            EXPECT_TRUE(attrsvar.find("unique") == attrsvar.end());
            EXPECT_EQ("xyz", attrsvar["foo"].get_string());
            EXPECT_EQ("1", copy.category()->id());
        }
    }

    // assignment copy
    {
        CategorisedResult result(cat);
        result.set_uri("uri a");
        result.set_title("title a");
        result.set_art("icon a");
        result.set_dnd_uri("dnd_uri a");
        CategorisedResult copy = result;

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
        EXPECT_EQ("bar", result.serialize()["attrs"].get_dict()["foo"].get_string());
        EXPECT_EQ("1", result.category()->id());

        EXPECT_EQ("uri b", copy.uri());
        EXPECT_EQ("title b", copy.title());
        EXPECT_EQ("icon b", copy.art());
        EXPECT_EQ("dnd_uri b", copy.dnd_uri());
        EXPECT_EQ("xyz", copy.serialize()["attrs"].get_dict()["foo"].get_string());
        EXPECT_EQ("1", copy.category()->id());
    }
}

// test conversion to VariantMap
TEST(CategorisedResult, serialize)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);

    CategorisedResult result(cat);
    result.set_uri("http://ubuntu.com");
    result.set_title("a title");
    result.set_art("an icon");
    result.set_dnd_uri("http://canonical.com");

    EXPECT_EQ("http://ubuntu.com", result.uri());
    EXPECT_EQ("a title", result.title());
    EXPECT_EQ("an icon", result.art());
    EXPECT_EQ("http://canonical.com", result.dnd_uri());

    auto outer_var = result.serialize();
    auto var = outer_var["attrs"].get_dict();
    EXPECT_EQ("http://ubuntu.com", var["uri"].get_string());
    EXPECT_EQ("a title", var["title"].get_string());
    EXPECT_EQ("an icon", var["art"].get_string());
    EXPECT_EQ("http://canonical.com", var["dnd_uri"].get_string());
    EXPECT_EQ("1", outer_var["internal"].get_dict()["cat_id"].get_string());
}

// test exceptions when converting to VariantMap
TEST(CategorisedResult, serialize_excp)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    CategorisedResult result(cat);

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
TEST(CategorisedResultItem, exceptions)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    Category::SCPtr null_cat;

    bool excp = false;
    try
    {
        CategorisedResult r(null_cat);
    }
    catch (const unity::InvalidArgumentException& e)
    {
        excp = true;
    }
    EXPECT_TRUE(excp);
}

// test conversion from VariantMap
TEST(CategorisedResult, deserialize)
{
    VariantMap vm;
    vm["uri"] = "http://ubuntu.com";
    vm["dnd_uri"] = "http://canonical.com";
    vm["title"] = "a title";
    vm["art"] = "an icon";
    vm["foo"] = "bar"; // custom attribute

    VariantMap intvm;
    intvm["cat_id"] = "2";

    VariantMap outer;
    outer["attrs"] = vm;
    outer["internal"] = intvm;

    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    CategorisedResult result(cat, outer);

    auto outer_var = result.serialize();
    auto var = outer_var["attrs"].get_dict();
    EXPECT_EQ("http://ubuntu.com", var["uri"].get_string());
    EXPECT_EQ("a title", var["title"].get_string());
    EXPECT_EQ("an icon", var["art"].get_string());
    EXPECT_EQ("http://canonical.com", var["dnd_uri"].get_string());
    EXPECT_EQ("bar", var["foo"].get_string());
    EXPECT_EQ("1", outer_var["internal"].get_dict()["cat_id"].get_string());
}

TEST(CategorisedResult, store)
{
    CategoryRenderer rdr;
    CategoryRegistry input_reg;
    CategoryRegistry output_reg;
    auto incat = input_reg.register_category("1", "title", "icon", rdr);
    auto outcat = output_reg.register_category("2", "title", "icon", rdr);

    CategorisedResult outresult(outcat);
    outresult.set_uri("uri1");
    outresult.set_title("title1");
    outresult.set_art("icon1");
    outresult.set_dnd_uri("dnduri1");

    EXPECT_THROW(outresult.store(outresult), unity::InvalidArgumentException); // cannot store self
    EXPECT_FALSE(outresult.has_stored_result());

    {
        CategorisedResult inresult(incat);
        inresult.set_uri("uri1");
        inresult.set_title("title1");
        inresult.set_art("icon1");
        inresult.set_dnd_uri("dnduri1");
        outresult.store(inresult);
    }

    EXPECT_TRUE(outresult.has_stored_result());

    EXPECT_EQ("uri1", outresult.uri());
    EXPECT_EQ("title1", outresult.title());
    EXPECT_EQ("icon1", outresult.art());
    EXPECT_EQ("dnduri1", outresult.dnd_uri());
    EXPECT_EQ("2", outresult.category()->id());

    auto inresult = outresult.retrieve();
    EXPECT_EQ("uri1", inresult.uri());
    EXPECT_EQ("title1", inresult.title());
    EXPECT_EQ("icon1", inresult.art());
    EXPECT_EQ("dnduri1", inresult.dnd_uri());
}
