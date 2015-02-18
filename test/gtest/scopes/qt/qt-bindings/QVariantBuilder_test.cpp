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

#include <gtest/gtest.h>

#include <unity/scopes/qt/QVariantBuilder.h>
#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/qt/internal/QVariantBuilderImpl.h>

#include <unity/scopes/VariantBuilder.h>

#include <iostream>

using namespace unity::scopes::qt::internal;
using namespace unity::scopes::qt;
using namespace unity::scopes;

TEST(QVariantBuilder, bindings)
{
    QVariantBuilderImpl* impl = new QVariantBuilderImpl();
    QVariantBuilder builder = QVariantBuilderImpl::create(impl);

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
    EXPECT_EQ(scopeVariantToQVariant(val3), val);
    EXPECT_EQ(val3, qVariantToScopeVariant(val));
}
