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
#include <scopes/PlacementHint.h>
#include <scopes/CategoryRenderer.h>
#include <scopes/internal/CategoryRegistry.h>
#include <unity/UnityExceptions.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(PlacementHint, basic)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("cat1", "A Category", "icon", rdr);
    {
        auto p = PlacementHint::search_bar_area();
        EXPECT_EQ(PlacementHint::Placement::SearchBarArea, p.placement());
    }
    {
        auto p = PlacementHint::annotation_area();
        EXPECT_EQ(PlacementHint::Placement::AnnotationArea, p.placement());
    }
    {
        auto p = PlacementHint::results_area(cat);
        EXPECT_EQ(PlacementHint::Placement::ResultsArea, p.placement());
    }
    {
        auto p = PlacementHint::category_area(cat);
        EXPECT_EQ(PlacementHint::Placement::CategoryArea, p.placement());
    }
}

TEST(PlacementHint, serialize)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("cat1", "A Category", "icon", rdr);
    {
        auto p = PlacementHint::search_bar_area();
        auto vm = p.serialize();
        EXPECT_EQ("searchbar", vm["area"].get_string());
        EXPECT_TRUE(vm.find("category") == vm.end());
    }
    {
        auto p = PlacementHint::annotation_area();
        auto vm = p.serialize();
        EXPECT_EQ("annotation", vm["area"].get_string());
        EXPECT_TRUE(vm.find("category") == vm.end());
    }
    {
        auto p = PlacementHint::results_area(cat);
        auto vm = p.serialize();
        EXPECT_EQ("results", vm["area"].get_string());
        EXPECT_EQ("cat1", vm["category"].get_string());
    }
    {
        auto p = PlacementHint::category_area(cat);
        auto vm = p.serialize();
        EXPECT_EQ("category", vm["area"].get_string());
        EXPECT_EQ("cat1", vm["category"].get_string());
    }
}

TEST(PlacementHint, exceptions)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("cat1", "A Category", "icon", rdr);
    {
        auto p = PlacementHint::search_bar_area();
        EXPECT_THROW(p.category(), unity::LogicException);
    }
    {
        auto p = PlacementHint::annotation_area();
        EXPECT_THROW(p.category(), unity::LogicException);
    }
}
