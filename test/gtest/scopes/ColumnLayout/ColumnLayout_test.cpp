/*
 * Copyright (C) 2014 Canonical Ltd
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

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/internal/ColumnLayoutImpl.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(ColumnLayout, basic)
{
    {
        ColumnLayout cl(1);
        EXPECT_EQ(1, cl.number_of_columns());
        EXPECT_EQ(0, cl.size());

        cl.add_column({"widget1", "widget2"});
        EXPECT_EQ(1, cl.number_of_columns());
        EXPECT_EQ(1, cl.size());
        EXPECT_EQ(2u, cl.column(0).size());
        EXPECT_EQ("widget1", cl.column(0)[0]);
    }
    {
        ColumnLayout cl(2);
        EXPECT_EQ(2, cl.number_of_columns());
        EXPECT_EQ(0, cl.size());

        cl.add_column({"widget1", "widget2"});
        cl.add_column({"widget3"});
        EXPECT_EQ(2, cl.number_of_columns());
        EXPECT_EQ(2, cl.size());
        EXPECT_EQ("widget1", cl.column(0)[0]);
        EXPECT_EQ("widget2", cl.column(0)[1]);
        EXPECT_EQ("widget3", cl.column(1)[0]);
    }
}

TEST(ColumnLayout, exceptions)
{
    {
        ColumnLayout cl(1);
        cl.add_column({"widget1", "widget2"});
        EXPECT_THROW(cl.column(1), unity::InvalidArgumentException);
        EXPECT_THROW(cl.add_column({"widget3"}), unity::LogicException);
        EXPECT_EQ(1, cl.size());
    }
    {
        VariantMap var;
        EXPECT_THROW(ColumnLayoutImpl::create(var), unity::InvalidArgumentException);
    }
}

TEST(ColumnLayout, serialize)
{
    {
        ColumnLayout cl(1);
        cl.add_column({"widget1", "widget2"});
        auto var = cl.serialize();
        auto outer = var["column_data"];
        EXPECT_EQ(1u, outer.get_array().size());
        auto inner = outer.get_array()[0];
        EXPECT_EQ(2u, inner.get_array().size());
    }
}

TEST(ColumnLayout, deserialize)
{
    {
        VariantArray outer;
        VariantArray inner({Variant("widget1"), Variant("widget2")});
        outer.push_back(Variant(inner));
        VariantMap var;
        var["column_data"] = Variant(outer);

        auto layout = ColumnLayoutImpl::create(var);
        EXPECT_EQ(1, layout.size());
        EXPECT_EQ(1, layout.number_of_columns());
        EXPECT_EQ(2u, layout.column(0).size());
        EXPECT_EQ("widget1", layout.column(0)[0]);
        EXPECT_EQ("widget2", layout.column(0)[1]);
    }
}
