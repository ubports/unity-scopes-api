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

#include <scopes/ActivationResponse.h>
#include <scopes/internal/ActivationResponseImpl.h>
#include <unity/UnityExceptions.h>
#include <gtest/gtest.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(ActivationResponse, basic)
{
    {
        ActivationResponse resp(ActivationResponse::Status::NotHandled);
        EXPECT_EQ(ActivationResponse::Status::NotHandled, resp.status());
        EXPECT_EQ(0, resp.hints().size());
    }
    {
        ActivationResponse resp(ActivationResponse::Status::ShowPreview);
        EXPECT_EQ(ActivationResponse::Status::ShowPreview, resp.status());
        EXPECT_EQ(0, resp.hints().size());
    }
    {
        ActivationResponse resp(ActivationResponse::Status::Handled);
        VariantMap var;
        var["foo"] = "bar";
        resp.setHints(var);
        EXPECT_EQ(ActivationResponse::Status::Handled, resp.status());
        EXPECT_EQ(1, resp.hints().size());
        EXPECT_EQ("bar", resp.hints()["foo"].get_string());
    }
}

TEST(ActivationResponse, serialize)
{
    ActivationResponse resp(ActivationResponse::Status::Handled);
    {
        VariantMap var;
        var["foo"] = "bar";
        resp.setHints(var);
    }
    auto var = resp.serialize();
    EXPECT_EQ(ActivationResponse::Status::Handled, static_cast<ActivationResponse::Status>(var["status"].get_int()));
    EXPECT_EQ("bar", var["hints"].get_dict()["foo"].get_string());
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
        var["hints"] = VariantMap();
        try
        {
            internal::ActivationResponseImpl::create(var);
            FAIL();
        }
        catch (unity::LogicException const &e)
        {
        }
    }

    // valid variant
    {
        VariantMap hints;
        hints["foo"] = "bar";
        VariantMap var;
        var["hints"] = hints;
        var["status"] = static_cast<int>(ActivationResponse::Status::Handled);
        try
        {
            internal::ActivationResponseImpl res(var);
            EXPECT_EQ("bar", res.hints()["foo"].get_string());
            EXPECT_EQ(ActivationResponse::Status::Handled, res.status());
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
        ActivationResponse resp(ActivationResponse::Status::Handled);
        ActivationResponse copy(resp);
        {
            VariantMap var;
            var["foo"] = "bar";
            resp.setHints(var);
        }
        {
            VariantMap var;
            var["iron"] = "maiden";
            copy.setHints(var);
        }
        EXPECT_EQ(ActivationResponse::Status::Handled, resp.status());
        EXPECT_EQ(ActivationResponse::Status::Handled, copy.status());
        EXPECT_EQ(1, resp.hints().size());
        EXPECT_EQ("bar", resp.hints()["foo"].get_string());
        EXPECT_EQ(1, copy.hints().size());
        EXPECT_EQ("maiden", copy.hints()["iron"].get_string());
    }
}

TEST(ActivationResponse, assign_op_copy)
{
    {
        ActivationResponse resp(ActivationResponse::Status::Handled);
        ActivationResponse copy = resp;
        {
            VariantMap var;
            var["foo"] = "bar";
            resp.setHints(var);
        }
        {
            VariantMap var;
            var["iron"] = "maiden";
            copy.setHints(var);
        }
        EXPECT_EQ(ActivationResponse::Status::Handled, resp.status());
        EXPECT_EQ(ActivationResponse::Status::Handled, copy.status());
        EXPECT_EQ(1, resp.hints().size());
        EXPECT_EQ("bar", resp.hints()["foo"].get_string());
        EXPECT_EQ(1, copy.hints().size());
        EXPECT_EQ("maiden", copy.hints()["iron"].get_string());
    }
}
