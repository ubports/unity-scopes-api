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

#include <gtest/gtest.h>
#include <unity/scopes/Query.h>
#include <unity/scopes/internal/QueryImpl.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

// basic test of Query setters and getters
TEST(Query, basic)
{
    {
        Query q("scope-A");
        EXPECT_EQ("scope-A", q.scope_id());
        EXPECT_EQ("", q.query_string());
        EXPECT_EQ("", q.department_id());
    }
    {
        Query q("scope-A");
        q.set_query_string("foo");
        q.set_department_id("dep1");
        EXPECT_EQ("scope-A", q.scope_id());
        EXPECT_EQ("foo", q.query_string());
        EXPECT_EQ("dep1", q.department_id());
    }
}

TEST(Query, copy)
{
    {
        Query a("scope-A", "foo", "dep1");
        Query b(a);

        EXPECT_EQ(a.scope_id(), b.scope_id());
        EXPECT_EQ(a.department_id(), b.department_id());
        EXPECT_EQ(a.query_string(), b.query_string());
        a.set_query_string("bar");
        a.set_department_id("dep2");

        EXPECT_EQ("foo", b.query_string());
        EXPECT_EQ("dep1", b.department_id());
    }
    {
        Query a("scope-A", "foo", "dep1");
        Query b = a;

        EXPECT_EQ(a.scope_id(), b.scope_id());
        EXPECT_EQ(a.department_id(), b.department_id());
        EXPECT_EQ(a.query_string(), b.query_string());
        a.set_query_string("bar");
        a.set_department_id("dep2");

        EXPECT_EQ("foo", b.query_string());
        EXPECT_EQ("dep1", b.department_id());
    }
}

// test of serialization into a canned query string
TEST(Query, to_string)
{
    {
        Query q("scope-A");
        q.set_query_string("foo");
        q.set_department_id("dep1");
        EXPECT_EQ("scope://scope-A?q=foo&department=dep1", q.to_string());
    }
    {
        Query q("scope-A");
        EXPECT_EQ("scope://scope-A?q=", q.to_string());
    }
}

TEST(Query, serialize)
{
    Query q("scope-A");
    q.set_query_string("foo");
    q.set_department_id("dep1");

    auto var = q.serialize();
    EXPECT_EQ("scope-A", var["scope"].get_string());
    EXPECT_EQ("dep1", var["department_id"].get_string());
    EXPECT_EQ("foo", var["query_string"].get_string());
    EXPECT_TRUE(var.find("filter_state") != var.end());
}

TEST(Query, deserialize)
{
    {
        VariantMap vm;
        vm["scope"] = "scope-A";
        vm["query_string"] = "foo";
        vm["department_id"] = "dep1";
        vm["filter_state"] = Variant(VariantMap());

        auto q = internal::QueryImpl::create(vm);
        EXPECT_EQ("scope-A", q.scope_id());
        EXPECT_EQ("foo", q.query_string());
        EXPECT_EQ("dep1", q.department_id());
    }
}

TEST(Query, exceptions)
{
    EXPECT_THROW(Query(""), unity::InvalidArgumentException);
    {
        VariantMap vm;
        try
        {
            // missing 'scope'
            internal::QueryImpl::create(vm);
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
            internal::QueryImpl::create(vm);
            FAIL();
        }
        catch (unity::InvalidArgumentException const& e)
        {
        }

        vm["scope"] = "scope-A";
        vm["filter_state"] = Variant(VariantMap());
        try
        {
            internal::QueryImpl::create(vm);
        }
        catch (unity::InvalidArgumentException const& e)
        {
            FAIL();
        }
    }
}
