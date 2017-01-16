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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/internal/CannedQueryImpl.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/FilterState.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

// basic test of CannedQuery setters and getters
TEST(CannedQuery, basic)
{
    {
        CannedQuery q("scope-A");
        EXPECT_EQ("scope-A", q.scope_id());
        EXPECT_EQ("", q.query_string());
        EXPECT_EQ("", q.department_id());
        EXPECT_FALSE(q.has_user_data());
    }
    {
        CannedQuery q("scope-A");
        q.set_query_string("foo");
        q.set_department_id("dep1");
        VariantArray va({Variant(10), Variant(20)});
        q.set_user_data(Variant(va));
        EXPECT_EQ("scope-A", q.scope_id());
        EXPECT_EQ("foo", q.query_string());
        EXPECT_EQ("dep1", q.department_id());
        EXPECT_TRUE(q.has_user_data());
        EXPECT_EQ(10, q.user_data().get_array()[0].get_int());
        EXPECT_EQ(20, q.user_data().get_array()[1].get_int());
    }
}

TEST(CannedQuery, copy)
{
    {
        CannedQuery a("scope-A", "foo", "dep1");
        a.set_user_data(Variant(1));
        CannedQuery b(a);

        EXPECT_EQ(a.scope_id(), b.scope_id());
        EXPECT_EQ(a.department_id(), b.department_id());
        EXPECT_EQ(a.query_string(), b.query_string());
        a.set_query_string("bar");
        a.set_department_id("dep2");
        a.set_user_data(Variant(2));

        EXPECT_EQ("foo", b.query_string());
        EXPECT_EQ("dep1", b.department_id());
        EXPECT_TRUE(b.has_user_data());
        EXPECT_EQ(1, b.user_data().get_int());

    }
    {
        CannedQuery a("scope-A", "foo", "dep1");
        a.set_user_data(Variant(1));
        CannedQuery b("scope-B");
        b = a;

        EXPECT_EQ(a.scope_id(), b.scope_id());
        EXPECT_EQ(a.department_id(), b.department_id());
        EXPECT_EQ(a.query_string(), b.query_string());
        a.set_query_string("bar");
        a.set_department_id("dep2");
        a.set_user_data(Variant(2));

        EXPECT_EQ("foo", b.query_string());
        EXPECT_EQ("dep1", b.department_id());
        EXPECT_TRUE(b.has_user_data());
        EXPECT_EQ(1, b.user_data().get_int());
    }
}

TEST(CannedQuery, move)
{
    {
        CannedQuery a("scope-A", "foo", "dep1");
        a.set_user_data(Variant(1));

        CannedQuery b(std::move(a));
        EXPECT_EQ("scope-A", b.scope_id());
        EXPECT_EQ("foo", b.query_string());
        EXPECT_EQ("dep1", b.department_id());
        EXPECT_EQ(1, b.user_data().get_int());
    }

    {
        CannedQuery a("scope-A", "foo", "dep1");
        CannedQuery b("scope-B");
        a.set_user_data(Variant(1));

        b = std::move(a);
        EXPECT_EQ("scope-A", b.scope_id());
        EXPECT_EQ("foo", b.query_string());
        EXPECT_EQ("dep1", b.department_id());
        EXPECT_EQ(1, b.user_data().get_int());
    }
}

// test of serialization into a canned query string
TEST(CannedQuery, to_uri)
{
    {
        CannedQuery q("scope-A");
        q.set_query_string("foo");
        q.set_department_id("dep1");
        EXPECT_EQ("scope://scope-A?q=foo&dep=dep1", q.to_uri());
    }
    {
        CannedQuery q("scope-A");
        q.set_query_string("foo");
        q.set_department_id("dep1");
        q.set_user_data(Variant(123));
        EXPECT_EQ("scope://scope-A?q=foo&dep=dep1&data=123%0A", q.to_uri());
    }
    {
        CannedQuery q("scope-A");
        EXPECT_EQ("scope://scope-A?q=", q.to_uri());
    }
    {
        CannedQuery q("scope-A");
        q.set_query_string("foo bar Baz");
        q.set_department_id("dep 1");
        EXPECT_EQ("scope://scope-A?q=foo%20bar%20Baz&dep=dep%201", q.to_uri());
    }
    {
        CannedQuery q("com.canonical.scope.foo");
        q.set_query_string("ß"); // utf8 character
        EXPECT_EQ("scope://com.canonical.scope.foo?q=%C3%9F", q.to_uri());
    }
    {
        {
            CannedQuery q("scopeA");
            q.set_query_string("foo");
            q.set_department_id("dep1");

            FilterState fstate;
            {
                auto filter = OptionSelectorFilter::create("f1", "Choose an option", false);
                auto option1 = filter->add_option("o1", "Option 1");
                filter->add_option("o2", "Option 2");
                filter->update_state(fstate, option1, true);
            }
            q.set_filter_state(fstate);
            // filters is {"f1":["o1"]}
            EXPECT_EQ("scope://scopeA?q=foo&dep=dep1&filters=%7B%22f1%22%3A%5B%22o1%22%5D%7D%0A", q.to_uri());
        }
    }
}

TEST(CannedQuery, from_uri)
{
    // invalid schema
    try
    {
        CannedQuery::from_uri("http://foo.com");
        FAIL();
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: CannedQuery::from_uri(): unsupported schema 'http://foo.com'", e.what());
    }

    // missing scope id
    try
    {
        CannedQuery::from_uri("scope://");
    }
    catch (unity::InvalidArgumentException const& e)
    {
        EXPECT_STREQ("unity::InvalidArgumentException: CannedQuery()::from_uri(): scope id is empty in 'scope://'", e.what());
    }

    // missing argument for percent-encoded value
    {
        try
        {
            CannedQuery::from_uri("scope://foo?q=%");
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("unity::InvalidArgumentException: Failed to decode key 'q' of uri 'scope://foo?q=%':\n"
                    "    unity::InvalidArgumentException: from_percent_encoding(): too few characters for percent-encoded value",
                    e.what());
        }
    }
    // missing character in percent-encoded value
    {
        try
        {
            CannedQuery::from_uri("scope://foo?q=%0");
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("unity::InvalidArgumentException: Failed to decode key 'q' of uri 'scope://foo?q=%0':\n"
                    "    unity::InvalidArgumentException: from_percent_encoding(): too few characters for percent-encoded value",
                    e.what());
        }
    }
    // non-hex value in percent-encoded value
    {
        try
        {
            CannedQuery::from_uri("scope://foo?dep=%qy");
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
            EXPECT_STREQ("unity::InvalidArgumentException: Failed to decode key 'dep' of uri 'scope://foo?dep=%qy':\n"
                    "    unity::InvalidArgumentException: from_percent_encoding(): unsupported conversion of 'qy':\n        stoi",
                    e.what());
        }
    }
    // wrong filters type
    {
        EXPECT_THROW(CannedQuery::from_uri("scope://foo?q=&filters=0"), unity::InvalidArgumentException);
    }
    {
        auto q = CannedQuery::from_uri("scope://foo");
        EXPECT_EQ("foo", q.scope_id());
        EXPECT_EQ("", q.query_string());
        EXPECT_EQ("", q.department_id());
    }
    {
        auto q = CannedQuery::from_uri("scope://foo?q=");
        EXPECT_EQ("foo", q.scope_id());
        EXPECT_EQ("", q.query_string());
        EXPECT_EQ("", q.department_id());
    }
    {
        auto q = CannedQuery::from_uri("scope://foo?q=Foo");
        EXPECT_EQ("foo", q.scope_id());
        EXPECT_EQ("Foo", q.query_string());
        EXPECT_EQ("", q.department_id());
    }
    {
        auto q = CannedQuery::from_uri("scope://foo?dep=a%20bc&q=Foo%20bar");
        EXPECT_EQ("foo", q.scope_id());
        EXPECT_EQ("Foo bar", q.query_string());
        EXPECT_EQ("a bc", q.department_id());
    }
    {
        auto q = CannedQuery::from_uri("scope://foo?q=Foo&data=%22bar%22%0A");
        EXPECT_EQ("foo", q.scope_id());
        EXPECT_EQ("Foo", q.query_string());
        EXPECT_EQ("bar", q.user_data().get_string());
    }

    // percent-encoded host supported for backwards compatibility
    {
        auto q = CannedQuery::from_uri("scope://com%2Ecanonical%2Escope%2Efoo?q=Foo&filters=%7B%22f1%22%3A%5B%22o1%22%5D%7D");
        EXPECT_EQ("com.canonical.scope.foo", q.scope_id());
        EXPECT_EQ("Foo", q.query_string());
        auto const fstate = q.filter_state();
        auto filter = OptionSelectorFilter::create("f1", "Choose an option", false);
        filter->add_option("o1", "Option 1");
        filter->add_option("o2", "Option 2");
        auto actopts = filter->active_options(fstate);
        EXPECT_EQ(1u, actopts.size());
        EXPECT_EQ("o1", (*(actopts.begin()))->id());
    }
    {
        auto q = CannedQuery::from_uri("scope://com.canonical.scope.foo?q=Foo");
        EXPECT_EQ("com.canonical.scope.foo", q.scope_id());
        EXPECT_EQ("Foo", q.query_string());
    }
    // no "q=" argument
    {
        auto q = CannedQuery::from_uri("scope://foo?dep=a");
        EXPECT_EQ("foo", q.scope_id());
        EXPECT_EQ("", q.query_string());
        EXPECT_EQ("a", q.department_id());
    }
    {
        auto q = CannedQuery::from_uri("scope://foo?dep=a%20bc&q=Foo%20bar");
        EXPECT_EQ("foo", q.scope_id());
        EXPECT_EQ("Foo bar", q.query_string());
        EXPECT_EQ("a bc", q.department_id());
    }
}

TEST(CannedQuery, serialize)
{
    CannedQuery q("scope-A");
    q.set_query_string("foo");
    q.set_department_id("dep1");
    q.set_user_data(Variant("x"));

    auto var = q.serialize();
    EXPECT_EQ("scope-A", var["scope"].get_string());
    EXPECT_EQ("dep1", var["department_id"].get_string());
    EXPECT_EQ("foo", var["query_string"].get_string());
    EXPECT_EQ("x", var["user_data"].get_string());
    EXPECT_TRUE(var.find("filter_state") != var.end());
}

TEST(CannedQuery, deserialize)
{
    {
        VariantMap vm;
        vm["scope"] = "scope-A";
        vm["query_string"] = "foo";
        vm["department_id"] = "dep1";
        vm["filter_state"] = Variant(VariantMap());
        vm["user_data"] = Variant(133);

        auto q = internal::CannedQueryImpl::create(vm);
        EXPECT_EQ("scope-A", q.scope_id());
        EXPECT_EQ("foo", q.query_string());
        EXPECT_EQ("dep1", q.department_id());
        EXPECT_EQ(133, q.user_data().get_int());
    }
}

TEST(CannedQuery, exceptions)
{
    EXPECT_THROW(CannedQuery(""), unity::InvalidArgumentException);
    EXPECT_THROW(CannedQuery("", "", ""), unity::InvalidArgumentException);
    {
        VariantMap vm;
        try
        {
            // missing 'scope'
            internal::CannedQueryImpl::create(vm);
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
        }

        vm["scope"] = "";
        vm["filter_state"] = Variant(VariantMap());
        try
        {
            // empty 'scope' not allowed
            internal::CannedQueryImpl::create(vm);
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
        }

        vm["scope"] = "scope-A";
        vm["filter_state"] = Variant(VariantMap());
        try
        {
            internal::CannedQueryImpl::create(vm);
        }
        catch (unity::InvalidArgumentException const& e)
        {
            FAIL();
        }
    }
    // no data
    {
        CannedQuery q("fooscope");
        EXPECT_THROW(q.user_data(), unity::LogicException);
    }
}
