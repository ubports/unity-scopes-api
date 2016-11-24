/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/qt/QCategorisedResult.h>
#include <unity/scopes/qt/QCategory.h>

#include <unity/scopes/qt/internal/QCategorisedResultImpl.h>
#include <unity/scopes/qt/internal/QCategoryImpl.h>
#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/internal/CategoryRegistry.h>

#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;
using namespace unity::scopes::qt;

class QCategorisedResult_test
{
public:
    static std::shared_ptr<QCategory> create_category(std::string const& id)
    {
        CategoryRegistry reg;
        CategoryRenderer rdr;
        auto cat = reg.register_category(id, "title", "icon", nullptr, rdr);

        return unity::scopes::qt::internal::QCategoryImpl::create(cat);
    }
};

TEST(QCategorisedResult, bindings)
{
    std::shared_ptr<QCategory const> qCategory = QCategorisedResult_test::create_category("1");

    QCategorisedResult result(qCategory);

    result.set_uri("test_uri");
    EXPECT_EQ(result.uri(), "test_uri");

    // test the [] operator
    EXPECT_EQ(result["uri"].toString(), "test_uri");
    result["test_attr"] = "test_value";
    EXPECT_EQ(result["test_attr"].toString(), "test_value");
    result["test_attr"] = "test_value2";
    EXPECT_EQ(result["test_attr"].toString(), "test_value2");

    // check the category stored
    EXPECT_EQ(result.category()->id(), qCategory->id());
    EXPECT_EQ(result.category()->title(), qCategory->title());
    EXPECT_EQ(result.category()->icon(), qCategory->icon());
    EXPECT_EQ(result.category()->query(), qCategory->query());
    EXPECT_EQ(result.category()->renderer_template(), qCategory->renderer_template());
    EXPECT_EQ(result.category()->serialize(), qCategory->serialize());

    // change category and check
    auto c2 = QCategorisedResult_test::create_category("2");
    result.set_category(c2);
    EXPECT_EQ("2", result.category()->id());
    result.set_category(qCategory);
    EXPECT_EQ("1", result.category()->id());

    // check the copy
    result["test_attr"] = "test_value3";
    QCategorisedResult result2(result);
    EXPECT_EQ(result2.category()->id(), qCategory->id());
    EXPECT_EQ(result2.category()->title(), qCategory->title());
    EXPECT_EQ(result2.category()->icon(), qCategory->icon());
    EXPECT_EQ(result2.category()->serialize(), qCategory->serialize());
    EXPECT_EQ(result2["test_attr"], "test_value3");
    EXPECT_EQ(result2.uri(), "test_uri");

    result = result2;
    EXPECT_EQ(result.category()->id(), qCategory->id());
    EXPECT_EQ(result.category()->title(), qCategory->title());
    EXPECT_EQ(result.category()->icon(), qCategory->icon());
    EXPECT_EQ(result.category()->serialize(), qCategory->serialize());
    EXPECT_EQ(result["test_attr"], "test_value3");
    EXPECT_EQ(result.uri(), "test_uri");

    {
        // Impl assignment coverage
        std::shared_ptr<QCategory const> cat1 = QCategorisedResult_test::create_category("1");
        unity::scopes::qt::internal::QCategorisedResultImpl ri1(cat1);

        std::shared_ptr<QCategory const> cat2 = QCategorisedResult_test::create_category("2");
        unity::scopes::qt::internal::QCategorisedResultImpl ri2(cat2);

        ri1 = ri2;
        EXPECT_EQ("2", ri1.category()->id());
    }
}

// Tests for QResult base class

TEST(QCategorisedResult, QResult)
{
    std::shared_ptr<QCategory const> out_cat = QCategorisedResult_test::create_category("1");
    std::shared_ptr<QCategory const> in_cat = QCategorisedResult_test::create_category("2");

    QCategorisedResult out_result(out_cat);
    out_result.set_uri("test_uri1");
    out_result.set_title("title1");
    out_result.set_art("art1");
    out_result.set_dnd_uri("dnd_uri1");
    out_result["key"] = "value";
    EXPECT_EQ("test_uri1", out_result.uri());
    EXPECT_EQ("title1", out_result.title());
    EXPECT_EQ("art1", out_result.art());
    EXPECT_EQ("dnd_uri1", out_result.dnd_uri());
    EXPECT_TRUE(out_result.contains("key"));
    EXPECT_EQ("value", out_result.value("key"));
    try
    {
        out_result.target_scope_proxy();
        FAIL();
    }
    catch (unity::LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: Result::target_scope_proxy(): undefined target scope", e.what());
    }

    EXPECT_TRUE(out_result.direct_activation());
    out_result.set_intercept_activation();
    EXPECT_FALSE(out_result.direct_activation());

    // TODO: No way to get JSON from Qvariant at the moment.
    std::string json = Variant(qt::internal::qvariantmap_to_variantmap(out_result.serialize())).serialize_json();
    std::string expected = std::string(R"({"attrs":{"art":"art1","dnd_uri":"dnd_uri1","key":"value","title":"title1","uri":"test_uri1"},"internal":{"cat_id":"1","flags":1}})") + "\n";
    EXPECT_EQ(expected, json);

    QCategorisedResult in_result(in_cat);
    in_result.set_uri("test_uri2");
    in_result.set_title("title2");

    EXPECT_FALSE(out_result.has_stored_result());
    out_result.store(in_result);
    EXPECT_TRUE(out_result.has_stored_result());

    QResult r = out_result.retrieve();
    EXPECT_EQ(in_result.uri(), r.uri());  // TODO: Can't use in_result == r here because retrive() slices the class :-(
                                          //       That's a flaw in the underlying API
    QResult const qvc(out_result);
    EXPECT_EQ("value", qvc["key"]);

    // Copy and assignment

    {
        QResult c(out_result);
        EXPECT_EQ("title1", c.title());
    }

    {
        QResult c(out_result);
        c = in_result;
        EXPECT_EQ("title2", c.title());
        c = c;
        EXPECT_EQ("title2", c.title());
    }

    {
        CategoryRegistry reg;
        CategoryRenderer rdr;
        auto cat = reg.register_category("1", "title", "icon", nullptr, rdr);
        CategorisedResult result(cat);
        result.set_title("t");

        // QResult construction from Result is protected.
        struct Tester : public QResult
        {
            Tester(Result const& result)
                : QResult(result)
            {
            }
        };

        Tester t(result);
        EXPECT_EQ("t", t.title());

        // QResultImpl construction from QResultImpl* is protected.
        struct ImplTester : public qt::internal::QResultImpl
        {
            ImplTester(QResultImpl* qri)
                : QResultImpl(qri)
            {
            }

            ImplTester(Result const& r)
                : QResultImpl(r)
            {
                EXPECT_EQ("t", r.title());
                qt::internal::QResultImpl* qrip = new QResultImpl(r);
                EXPECT_EQ("t", qrip->title());

                // This is the real test to see that this constructor works
                ImplTester copy(qrip);
                EXPECT_EQ("t", copy.title());
                delete qrip;
            }
        };

        ImplTester it(result);
    }
}
