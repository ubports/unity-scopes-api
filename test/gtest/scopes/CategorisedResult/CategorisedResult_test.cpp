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

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/CategoryRegistry.h>
#include <unity/scopes/internal/ReplyObject.h>
#include <unity/scopes/internal/CategorisedResultImpl.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

// basic test of CategorisedResult setters and getters
TEST(CategorisedResult, basic)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
    auto cat2 = reg.register_category("2", "title", "icon", nullptr, rdr);

    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_title("a title");
        result.set_art("an icon");
        result.set_dnd_uri("http://canonical.com");

        EXPECT_EQ("http://ubuntu.com", result.uri());
        EXPECT_EQ("a title", result.title());
        EXPECT_EQ("an icon", result.art());
        EXPECT_EQ("http://canonical.com", result.dnd_uri());
        EXPECT_EQ("http://ubuntu.com", result.value("uri").get_string());
        EXPECT_EQ("a title", result.value("title").get_string());
        EXPECT_EQ("an icon", result.value("art").get_string());
        EXPECT_EQ("http://canonical.com", result.dnd_uri());

        EXPECT_EQ("1", result.category()->id());

        result.set_category(cat2);
        EXPECT_EQ("2", result.category()->id());
    }
}

// test for [] operator
TEST(CategorisedResult, indexop)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
    {
        CategorisedResult result(cat);
        result["uri"] = Variant("http://ubuntu.com");
        result["title"] = Variant("a title");
        result["art"] = "an icon";
        result["dnd_uri"] = "http://canonical.com";
        result["foo"] = Variant("bar");

        // referencing non-existing attribute creates it
        EXPECT_NO_THROW(result["nonexisting"]);
        EXPECT_TRUE(result.contains("nonexisting"));
        EXPECT_EQ(Variant::Type::Null, result["nonexisting"].which());

        EXPECT_EQ("http://ubuntu.com", result.uri());
        EXPECT_EQ("http://ubuntu.com", result["uri"].get_string());
        EXPECT_EQ("a title", result.title());
        EXPECT_EQ("a title", result["title"].get_string());
        EXPECT_EQ("an icon", result.art());
        EXPECT_EQ("an icon", result["art"].get_string());
        EXPECT_EQ("http://canonical.com", result.dnd_uri());
        EXPECT_EQ("http://canonical.com", result["dnd_uri"].get_string());
        EXPECT_TRUE(result.contains("foo"));
        EXPECT_EQ("bar", result["foo"].get_string());
        EXPECT_EQ("bar", result.value("foo").get_string());
        EXPECT_EQ("bar", result.serialize()["attrs"].get_dict()["foo"].get_string());
        EXPECT_EQ("1", result.category()->id());
    }
    {
        CategorisedResult result(cat);
        result["uri"] = Variant("http://ubuntu.com");

        // force const operator[]
        const CategorisedResult result2 = result;
        EXPECT_EQ("http://ubuntu.com", result2["uri"].get_string());
        // referencing non-existing attribute of const result object throws
        EXPECT_THROW(result2["nonexisting"], unity::InvalidArgumentException);
        EXPECT_FALSE(result2.contains("nonexisting"));
    }
}

TEST(CategorisedResult, compare)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);

    {
        CategorisedResult result1(cat);
        result1.set_uri("uri a");
        result1.set_title("title a");
        result1.set_art("icon a");
        result1["common"] = "o";

        CategorisedResult result2(cat);
        result2.set_uri("uri a");
        result2.set_title("title a");
        result2.set_art("icon a");
        result2["common"] = "o";

        EXPECT_TRUE(result1 == result2);

        CategorisedResult result3(cat);
        result3.set_uri("uri b");
        result3.set_title("title a");
        result3.set_art("icon a");
        result3["common"] = "o";

        EXPECT_FALSE(result1 == result3);
        EXPECT_FALSE(result2 == result3);
    }
}

TEST(CategorisedResult, copy)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
    // copy ctor
    {
        CategorisedResult result(cat);
        result.set_uri("uri a");
        result.set_title("title a");
        result.set_art("icon a");
        result.set_dnd_uri("dnd_uri a");
        result["common"] = "o";

        CategorisedResult copy(result);

        copy.set_uri("uri b");
        copy.set_title("title b");
        copy.set_art("icon b");
        copy.set_dnd_uri("dnd_uri b");

        result["foo"] = Variant("bar");
        result["unique"] = Variant(123);
        copy["foo"] = Variant("xyz");
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

        result["foo"] = Variant("bar");
        copy["foo"] = Variant("xyz");

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

        CategorisedResult copy2(cat);
        copy2 = copy;
        EXPECT_EQ("uri b", copy2.uri());
        EXPECT_EQ("title b", copy2.title());
        EXPECT_EQ("icon b", copy2.art());
        EXPECT_EQ("dnd_uri b", copy2.dnd_uri());
        EXPECT_EQ("xyz", copy2.serialize()["attrs"].get_dict()["foo"].get_string());
        EXPECT_EQ("1", copy2.category()->id());
    }
}

// test conversion to VariantMap
TEST(CategorisedResult, serialize)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
    {
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
    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_dnd_uri("http://canonical.com");

        auto outer_var = result.serialize();
        auto var = outer_var["attrs"].get_dict();
        EXPECT_EQ("http://ubuntu.com", var["uri"].get_string());
        EXPECT_TRUE(var.find("title") == var.end());
        EXPECT_TRUE(var.find("art") == var.end());
        EXPECT_EQ("http://canonical.com", var["dnd_uri"].get_string());
        EXPECT_EQ("1", outer_var["internal"].get_dict()["cat_id"].get_string());
    }

}

// test exceptions when converting to VariantMap
TEST(CategorisedResult, serialize_excp)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
    CategorisedResult result(cat);

    {
        // throw until mandatory 'uri' attribute is not empty
        EXPECT_THROW(result.serialize(), unity::InvalidArgumentException);
        result.set_uri("http://ubuntu.com");
        EXPECT_NO_THROW(result.serialize());
    }
    {
        result["uri"] = Variant(0);
        EXPECT_THROW(result.serialize(), unity::InvalidArgumentException);
        result["uri"] = "http://ubuntu.com";
        result["dnd_uri"] = Variant(0);
        EXPECT_THROW(result.serialize(), unity::InvalidArgumentException);
        result["dnd_uri"] = "http://ubuntu.com";
        EXPECT_NO_THROW(result.serialize());
    }
}

TEST(CategorisedResult, exceptions)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
    Category::SCPtr null_cat;
    {
        try
        {
            CategorisedResult r(null_cat);
            FAIL();
        }
        catch (const unity::InvalidArgumentException& e) {}
    }
    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        EXPECT_THROW(result.value("nonexisting"), unity::InvalidArgumentException);
    }
    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_dnd_uri("http://canonical.com");
        EXPECT_THROW(result.set_category(nullptr), unity::InvalidArgumentException);
    }
    {
        CategorisedResult result(cat);
        result.set_uri("http://ubuntu.com");
        result.set_title("a title");
        result.set_art("an icon");
        result.set_dnd_uri("http://canonical.com");
    }
}

// test conversion from VariantMap
TEST(CategorisedResult, deserialize)
{
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
        auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
        auto result = internal::CategorisedResultImpl::create_result(new CategorisedResultImpl(cat, outer));

        auto outer_var = result.serialize();
        auto var = outer_var["attrs"].get_dict();
        EXPECT_EQ("http://ubuntu.com", var["uri"].get_string());
        EXPECT_EQ("a title", var["title"].get_string());
        EXPECT_EQ("an icon", var["art"].get_string());
        EXPECT_EQ("http://canonical.com", var["dnd_uri"].get_string());
        EXPECT_EQ("bar", var["foo"].get_string());
        EXPECT_EQ("1", outer_var["internal"].get_dict()["cat_id"].get_string());
    }
    {
        VariantMap vm;
        vm["uri"] = "http://ubuntu.com";
        vm["dnd_uri"] = "http://canonical.com";
        vm["foo"] = "bar"; // custom attribute

        VariantMap intvm;
        intvm["cat_id"] = "2";

        VariantMap outer;
        outer["attrs"] = vm;
        outer["internal"] = intvm;

        CategoryRegistry reg;
        CategoryRenderer rdr;
        auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
        auto result = internal::CategorisedResultImpl::create_result(new internal::CategorisedResultImpl(cat, outer));

        EXPECT_EQ("http://ubuntu.com", result.uri());
        EXPECT_EQ("http://canonical.com", result.dnd_uri());
        EXPECT_EQ("", result.title());
        EXPECT_EQ("", result.art());
        EXPECT_FALSE(result.contains("title"));
        EXPECT_FALSE(result.contains("art"));
        EXPECT_EQ("bar", result["foo"].get_string());
    }
}

TEST(CategorisedResult, store)
{
    CategoryRenderer rdr;
    CategoryRegistry input_reg;
    CategoryRegistry output_reg;
    auto incat = input_reg.register_category("1", "title", "icon", nullptr, rdr);
    auto outcat = output_reg.register_category("2", "title", "icon", nullptr, rdr);

    CategorisedResult outresult(outcat);
    outresult.set_uri("uri2");
    outresult.set_title("title2");
    outresult.set_art("icon2");
    outresult.set_dnd_uri("dnduri2");

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

    EXPECT_EQ("uri2", outresult.uri());
    EXPECT_EQ("title2", outresult.title());
    EXPECT_EQ("icon2", outresult.art());
    EXPECT_EQ("dnduri2", outresult.dnd_uri());
    EXPECT_EQ("2", outresult.category()->id());

    auto inresult = outresult.retrieve();
    EXPECT_EQ("uri1", inresult.uri());
    EXPECT_EQ("title1", inresult.title());
    EXPECT_EQ("icon1", inresult.art());
    EXPECT_EQ("dnduri1", inresult.dnd_uri());
}
