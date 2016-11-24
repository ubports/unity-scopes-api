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

#include <unity/scopes/qt/QVariantBuilder.h>
#include <unity/scopes/qt/internal/QVariantBuilderImpl.h>
#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/scopes/VariantBuilder.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes::qt::internal;
using namespace unity::scopes::qt;
using namespace unity::scopes;

TEST(QVariantBuilder, constructors)
{
    QVariantBuilder b;
    b.add_tuple({{"int", QVariant(5)}});
    QVariantBuilder b2(b);
    auto v = b.end();
    auto v2 = b2.end();
    EXPECT_EQ(5, v.toList().at(0).toMap().value("int").toInt());
    EXPECT_EQ(v, v2);
}

TEST(QVariantBuilder, assignment)
{
    QVector<QPair<QString, QVariant>> vec;
    QPair<QString, QVariant> p{"int", QVariant(5)};
    vec.push_back({"int", QVariant(5)});
    QVariantBuilder b;
    b.add_tuple(vec);
    QVariantBuilder b2;
    b2 = b;
    auto v = b.end();
    auto v2 = b2.end();
    EXPECT_EQ(5, v.toList().at(0).toMap().value("int").toInt());
    EXPECT_EQ(v, v2);
}

TEST(QVariantBuilder, bindings)
{
    QVariantBuilderImpl* impl = new QVariantBuilderImpl();
    QVariantBuilder builder = QVariantBuilderImpl::create(impl);
    EXPECT_NE(nullptr, impl->get_api());

    builder.add_tuple({{"value", QVariant("attribute 1")}});
    builder.add_tuple({{"value", QVariant("attribute 2")}});
    builder.add_tuple({{"value", QVariant("attribute 3")}});
    QVariant val = builder.end();

    VariantBuilder builder3;
    builder3.add_tuple({{"value", Variant("attribute 1")}});
    builder3.add_tuple({{"value", Variant("attribute 2")}});
    builder3.add_tuple({{"value", Variant("attribute 3")}});
    Variant val3 = builder3.end();

    // check that the result using the Qt inter
    EXPECT_EQ(variant_to_qvariant(val3), val);
    EXPECT_EQ(val3, qvariant_to_variant(val));
}

TEST(QVariantBuilder, exceptions)
{
    QVariantBuilder b;
    try
    {
        b.end();
    }
    catch (unity::LogicException const& e)
    {
        EXPECT_STREQ("unity::LogicException: VariantBuilder::end(): no Variant has been constructed", e.what());
    }
}
