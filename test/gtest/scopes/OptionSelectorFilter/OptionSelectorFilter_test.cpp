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
#include <scopes/FilterState.h>
#include <scopes/OptionSelectorFilter.h>
#include <unity/UnityExceptions.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(OptionSelectorFilter, basic)
{
    OptionSelectorFilter filter1("f1", "Options", false);
    EXPECT_EQ("f1", filter1.id());
    EXPECT_EQ("Options", filter1.label());
    EXPECT_FALSE(filter1.multi_select());

    filter1.add_option("1", "Option 1");
    filter1.add_option("2", "Option 2");

    auto opts = filter1.options();
    EXPECT_EQ(2, opts.size());
    EXPECT_EQ("1", opts.front()->id());
    EXPECT_EQ("Option 1", opts.front()->label());
}

TEST(OptionSelectorFilter, state_single_selection)
{
    OptionSelectorFilter filter1("f1", "Options", false);
    auto option1 = filter1.add_option("1", "Option 1");
    filter1.add_option("2", "Option 2");

    FilterState fstate;
    EXPECT_FALSE(fstate.has_filter("f1"));

    // enable option1
    filter1.update_state(fstate, option1, true);
    EXPECT_TRUE(fstate.has_filter("f1"));
    auto active = filter1.active_options(fstate);
    EXPECT_EQ(1, active.size());
    EXPECT_TRUE(active.find(option1) != active.end());

    // disable option1; filter state remains in the FilterState, just no options are selected
    filter1.update_state(fstate, option1, false);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_EQ(0, filter1.active_options(fstate).size());
}

TEST(OptionSelectorFilter, state_multi_selection)
{
    OptionSelectorFilter filter1("f1", "Options", true);
    auto option1 = filter1.add_option("1", "Option 1");
    auto option2 = filter1.add_option("2", "Option 2");

    FilterState fstate;

    // enable option1
    filter1.update_state(fstate, option1, true);
    filter1.update_state(fstate, option2, true);
    EXPECT_TRUE(fstate.has_filter("f1"));
    auto active = filter1.active_options(fstate);
    EXPECT_EQ(2, active.size());
    EXPECT_TRUE(active.find(option1) != active.end());
    EXPECT_TRUE(active.find(option2) != active.end());

    // disable option1
    filter1.update_state(fstate, option1, false);
    EXPECT_EQ(1, filter1.active_options(fstate).size());
    EXPECT_TRUE(active.find(option1) != active.end());

    // disable option2
    filter1.update_state(fstate, option2, false);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_EQ(0, filter1.active_options(fstate).size());
}
