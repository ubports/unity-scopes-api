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
#include <unity/scopes/internal/ResultImpl.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

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
    {
        VariantMap vm;
        VariantMap attrs;
        attrs["uri"] = "foo";
        vm["internal"] = Variant(VariantMap());
        vm["attrs"] = Variant(attrs);
        auto const res = ResultImpl::create_result(vm);

        ActivationResponse resp(res);
        EXPECT_EQ(ActivationResponse::Status::UpdateResult, resp.status());
        EXPECT_EQ(res.uri(), resp.updated_result().uri());
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

    // UpdateResult only allowed with Result
    {
        try
        {
            ActivationResponse resp(ActivationResponse::Status::UpdateResult);
            FAIL();
        }
        catch (unity::InvalidArgumentException const&) {}
    }

    // UpdatePreview only allowed with PreviewWidgetList
    {
        try
        {
            ActivationResponse resp(ActivationResponse::Status::UpdatePreview);
            FAIL();
        }
        catch (unity::InvalidArgumentException const&) {}
    }
}

TEST(ActivationResponse, with_updated_widgets)
{
    {
        PreviewWidget w("w1", "foo");
        PreviewWidgetList const widgets {w};

        ActivationResponse resp(widgets);
        EXPECT_EQ(ActivationResponse::Status::UpdatePreview, resp.status());
        EXPECT_EQ(1u, resp.updated_widgets().size());
        EXPECT_EQ("w1", resp.updated_widgets().begin()->id());
    }

    {
        PreviewWidgetList const widgets;

        try
        {
            ActivationResponse resp(widgets);
            FAIL();
        }
        catch (unity::InvalidArgumentException const&)
        {
        }
    }
}

TEST(ActivationResponse, serialize)
{
    // just to make ResultImpl::set_runtime() happy, runtime must not be null.
    auto runtime = internal::RuntimeImpl::create("fooscope", TEST_DIR "/Runtime.ini");

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
    {
        VariantMap vm;
        VariantMap attrs;
        attrs["uri"] = Variant("foo");
        vm["internal"] = Variant(VariantMap());
        vm["attrs"] = Variant(attrs);
        auto const res = ResultImpl::create_result(vm);

        ActivationResponse resp(res);
        auto var = resp.serialize();
        EXPECT_EQ(ActivationResponse::Status::UpdateResult, static_cast<ActivationResponse::Status>(var["status"].get_int()));
        EXPECT_EQ("foo", var["updated_result"].get_dict()["attrs"].get_dict()["uri"].get_string());
    }
    {
        PreviewWidget const w("w1", "foo");
        PreviewWidgetList const widgets {w};

        ActivationResponse resp(widgets);
        auto var = resp.serialize();

        ActivationResponse resp2 = internal::ActivationResponseImpl::create(var, runtime.get());
        EXPECT_EQ(ActivationResponse::Status::UpdatePreview, resp2.status());
        EXPECT_EQ(1u, resp2.updated_widgets().size());
        EXPECT_EQ("w1", resp2.updated_widgets().begin()->id());
    }
}

TEST(ActivationResponse, deserialize)
{
    // just to make ResultImpl::set_runtime() happy, runtime must not be null.
    auto runtime = internal::RuntimeImpl::create("fooscope", TEST_DIR "/Runtime.ini");

    // invalid variant
    {
        VariantMap var;
        try
        {
            internal::ActivationResponseImpl res(var, runtime.get());
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
            internal::ActivationResponseImpl::create(var, runtime.get());
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
            internal::ActivationResponseImpl res(var, runtime.get());
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
            internal::ActivationResponseImpl res(var, runtime.get());
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
            internal::ActivationResponseImpl res(var, runtime.get());
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
            internal::ActivationResponseImpl res(var, runtime.get());
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
