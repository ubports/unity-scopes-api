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
#include <scopes/Hyperlink.h>
#include <scopes/PlacementHint.h>
#include <unity/UnityExceptions.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(Hyperlink, basic)
{
    {
        Query query("scope-A", "foo", "dep1");
        Hyperlink link(query, PlacementHint::search_bar_area());

        EXPECT_EQ(PlacementHint::Placement::SearchBarArea, link.placement_hint().placement());
        EXPECT_EQ("scope-A", link.canned_query().scope_name());
        EXPECT_EQ("foo", link.canned_query().query_string());
        EXPECT_EQ("dep1", link.canned_query().department_id());
    }
}

TEST(Hyperlink, serialize)
{
    {
        Query query("scope-A", "foo", "dep1");
        Hyperlink link(query, PlacementHint::search_bar_area());

        auto vm = link.serialize();
        EXPECT_EQ("hyperlink", vm["type"].get_string());
        EXPECT_EQ("searchbar", vm["placement"].get_dict()["area"].get_string());
        auto q = link.canned_query();
        EXPECT_EQ(q, query);
    }
}
