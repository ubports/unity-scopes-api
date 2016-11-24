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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/FilterState.h>
#include <unity/scopes/RadioButtonsFilter.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::experimental;
using namespace unity::scopes::internal;

TEST(RadioButtonsFilter, basic)
{
    auto filter1 = RadioButtonsFilter::create("f1", "Choose");
    EXPECT_EQ("f1", filter1->id());
    EXPECT_EQ("Choose", filter1->label());
    EXPECT_EQ(0u, filter1->options().size());

    filter1->add_option("a", "A");
    filter1->add_option("b", "B");

    auto opts = filter1->options();
    EXPECT_EQ(2u, opts.size());
    EXPECT_EQ("a", opts.front()->id());
    EXPECT_EQ("A", opts.front()->label());
    EXPECT_EQ("b", opts.back()->id());
    EXPECT_EQ("B", opts.back()->label());
}

TEST(RadioButtonsFilter, selection)
{
    auto filter1 = RadioButtonsFilter::create("f1", "Options");
    auto option1 = filter1->add_option("1", "Option 1");
    auto option2 = filter1->add_option("2", "Option 2");

    FilterState fstate;
    EXPECT_FALSE(fstate.has_filter("f1"));
    EXPECT_EQ(nullptr, filter1->active_option(fstate));
    EXPECT_FALSE(filter1->has_active_option(fstate));

    // enable option1
    filter1->update_state(fstate, option1, true);
    EXPECT_TRUE(fstate.has_filter("f1"));
    auto active = filter1->active_option(fstate);
    EXPECT_EQ("1", active->id());

    // enable option2, option1 get disabled
    filter1->update_state(fstate, option2, true);
    active = filter1->active_option(fstate);
    EXPECT_EQ("2", active->id());

    // disable option1; filter state remains in the FilterState, just no options are selected
    filter1->update_state(fstate, option2, false);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_EQ(nullptr, filter1->active_option(fstate));
}

TEST(RadioButtonsFilter, display_hints)
{
    auto filter = RadioButtonsFilter::create("f1", "Options");
    EXPECT_THROW(filter->set_display_hints(FilterBase::DisplayHints::Primary), unity::InvalidArgumentException);
}
