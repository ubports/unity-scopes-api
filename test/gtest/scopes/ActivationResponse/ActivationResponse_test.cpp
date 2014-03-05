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

#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/internal/ActivationResponseImpl.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(ActivationResponse, basic)
{
    {
        ActivationResponse resp(ActivationResponse::Status::NotHandled);
        EXPECT_EQ(ActivationResponse::Status::NotHandled, resp.status());
        EXPECT_TRUE(resp.scope_data().is_null());
    }
    {
        ActivationResponse resp(ActivationResponse::Status::ShowPreview);
        EXPECT_EQ(ActivationResponse::Status::ShowPreview, resp.status());
        EXPECT_TRUE(resp.scope_data().is_null());
    }
    {
        ActivationResponse resp(ActivationResponse::Status::HideDash);
        VariantMap var;
        var["foo"] = "bar";
        resp.set_scope_data(Variant(var));
        EXPECT_EQ(ActivationResponse::Status::HideDash, resp.status());
        EXPECT_EQ(1u, resp.scope_data().get_dict().size());
        EXPECT_EQ("bar", resp.scope_data().get_dict()["foo"].get_string());
        EXPECT_THROW(resp.query(), unity::LogicException);
    }

    {
        CannedQuery const query("scope-foo");
        ActivationResponse resp(query);
        EXPECT_EQ(ActivationResponse::Status::PerformQuery, resp.status());
    }

    // Search only allowed with CannedQuery
    {
        try
        {
            ActivationResponse resp(ActivationResponse::Status::PerformQuery);
            FAIL();
        }
        catch (unity::InvalidArgumentException const&) {}
    }
}

TEST(ActivationResponse, serialize)
{
    {
        ActivationResponse resp(ActivationResponse::Status::HideDash);
        {
            VariantMap var;
            var["foo"] = "bar";
            resp.set_scope_data(Variant(var));
        }
        auto var = resp.serialize();
        EXPECT_EQ(ActivationResponse::Status::HideDash, static_cast<ActivationResponse::Status>(var["status"].get_int()));
        EXPECT_EQ("bar", var["scope_data"].get_dict()["foo"].get_string());
    }
    {
        CannedQuery const query("scope-foo");
        ActivationResponse resp(query);
        auto var = resp.serialize();
        EXPECT_EQ(ActivationResponse::Status::PerformQuery, static_cast<ActivationResponse::Status>(var["status"].get_int()));
        EXPECT_EQ("scope-foo", var["query"].get_dict()["scope"].get_string());
    }
}

TEST(ActivationResponse, deserialize)
{
    // invalid variant
    {
        VariantMap var;
        try
        {
            internal::ActivationResponseImpl res(var);
            FAIL();
        }
        catch (unity::LogicException const &e)
        {
        }
    }

    // invalid variant - missing 'status'
    {
        VariantMap var;
        var["scope_data"] = VariantMap();
        try
        {
            internal::ActivationResponseImpl::create(var);
            FAIL();
        }
        catch (unity::LogicException const &e)
        {
        }
    }

    // invalid variant - missing 'query'
    {
        VariantMap hints;
        hints["foo"] = "bar";
        VariantMap var;
        var["scope_data"] = hints;
        var["status"] = static_cast<int>(ActivationResponse::Status::PerformQuery);
        try
        {
            internal::ActivationResponseImpl res(var);
            FAIL();
        }
        catch (unity::LogicException const &e) {}
    }

    // valid variant
    {
        VariantMap hints;
        hints["foo"] = "bar";
        VariantMap var;
        var["scope_data"] = hints;
        var["status"] = static_cast<int>(ActivationResponse::Status::HideDash);
        try
        {
            internal::ActivationResponseImpl res(var);
            EXPECT_EQ("bar", res.scope_data().get_dict()["foo"].get_string());
            EXPECT_EQ(ActivationResponse::Status::HideDash, res.status());
        }
        catch (unity::LogicException const &e)
        {
            FAIL();
        }
    }

    // valid variant
    {
        CannedQuery query("scope-foo");
        VariantMap var;
        var["scope_data"] = VariantMap();
        var["status"] = static_cast<int>(ActivationResponse::Status::PerformQuery);
        var["query"] = query.serialize();
        try
        {
            internal::ActivationResponseImpl res(var);
            EXPECT_EQ(ActivationResponse::Status::PerformQuery, res.status());
            EXPECT_EQ("scope-foo", res.query().scope_id());
        }
        catch (unity::LogicException const &e)
        {
            FAIL();
        }
    }

    // valid variant
    {
        VariantMap var;
        var["scope_data"] = Variant("foobar");
        var["status"] = static_cast<int>(ActivationResponse::Status::HideDash);
        try
        {
            internal::ActivationResponseImpl res(var);
            EXPECT_EQ("foobar", res.scope_data().get_string());
            EXPECT_EQ(ActivationResponse::Status::HideDash, res.status());
        }
        catch (unity::LogicException const &e)
        {
            FAIL();
        }
    }

}

TEST(ActivationResponse, copy_ctor)
{
    {
        ActivationResponse resp(ActivationResponse::Status::HideDash);
        ActivationResponse copy(resp);
        {
            VariantMap var;
            var["foo"] = "bar";
            resp.set_scope_data(Variant(var));
        }
        {
            VariantMap var;
            var["iron"] = "maiden";
            copy.set_scope_data(Variant(var));
        }
        EXPECT_EQ(ActivationResponse::Status::HideDash, resp.status());
        EXPECT_EQ(ActivationResponse::Status::HideDash, copy.status());
        EXPECT_EQ(1u, resp.scope_data().get_dict().size());
        EXPECT_EQ("bar", resp.scope_data().get_dict()["foo"].get_string());
        EXPECT_EQ(1u, copy.scope_data().get_dict().size());
        EXPECT_EQ("maiden", copy.scope_data().get_dict()["iron"].get_string());
    }
}

TEST(ActivationResponse, assign_op_copy)
{
    {
        ActivationResponse resp(ActivationResponse::Status::HideDash);
        ActivationResponse copy = resp;
        {
            VariantMap var;
            var["foo"] = "bar";
            resp.set_scope_data(Variant(var));
        }
        {
            VariantMap var;
            var["iron"] = "maiden";
            copy.set_scope_data(Variant(var));
        }
        EXPECT_EQ(ActivationResponse::Status::HideDash, resp.status());
        EXPECT_EQ(ActivationResponse::Status::HideDash, copy.status());
        EXPECT_EQ(1u, resp.scope_data().get_dict().size());
        EXPECT_EQ("bar", resp.scope_data().get_dict()["foo"].get_string());
        EXPECT_EQ(1u, copy.scope_data().get_dict().size());
        EXPECT_EQ("maiden", copy.scope_data().get_dict()["iron"].get_string());
    }
}
