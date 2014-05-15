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
#include <unity/scopes/FilterState.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/internal/OptionSelectorFilterImpl.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(OptionSelectorFilter, basic)
{
    auto filter1 = OptionSelectorFilter::create("f1", "Options", false);
    EXPECT_EQ("f1", filter1->id());
    EXPECT_EQ("Options", filter1->label());
    EXPECT_FALSE(filter1->multi_select());
    EXPECT_EQ(FilterBase::DisplayHints::Default, static_cast<FilterBase::DisplayHints>(filter1->display_hints()));

    filter1->set_display_hints(FilterBase::DisplayHints::Primary);
    filter1->add_option("1", "Option 1");
    filter1->add_option("2", "Option 2");

    EXPECT_EQ(FilterBase::DisplayHints::Primary, static_cast<FilterBase::DisplayHints>(filter1->display_hints()));
    auto opts = filter1->options();
    EXPECT_EQ(2u, opts.size());
    EXPECT_EQ("1", opts.front()->id());
    EXPECT_EQ("Option 1", opts.front()->label());
    EXPECT_EQ("2", opts.back()->id());
    EXPECT_EQ("Option 2", opts.back()->label());
}

TEST(OptionSelectorFilter, display_hints_range_check)
{
    auto filter1 = OptionSelectorFilter::create("f1", "Options", false);
    EXPECT_THROW(filter1->set_display_hints(-1), unity::InvalidArgumentException);
    // this check ensures we throw if this value exceeds the combination of FilterBase::DisplayHints flags.
    EXPECT_THROW(filter1->set_display_hints(2), unity::InvalidArgumentException);
}

TEST(OptionSelectorFilter, single_selection)
{
    auto filter1 = OptionSelectorFilter::create("f1", "Options", false);
    auto option1 = filter1->add_option("1", "Option 1");
    auto option2 = filter1->add_option("2", "Option 2");

    FilterState fstate;
    EXPECT_FALSE(fstate.has_filter("f1"));

    // enable option1
    filter1->update_state(fstate, option1, true);
    EXPECT_TRUE(fstate.has_filter("f1"));
    auto active = filter1->active_options(fstate);
    EXPECT_EQ(1u, active.size());
    EXPECT_TRUE(active.find(option1) != active.end());

    // enable option2, option1 get disabled
    filter1->update_state(fstate, option2, true);
    active = filter1->active_options(fstate);
    EXPECT_EQ(1u, active.size());
    EXPECT_TRUE(active.find(option2) != active.end());

    // disable option1; filter state remains in the FilterState, just no options are selected
    filter1->update_state(fstate, option2, false);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_EQ(0u, filter1->active_options(fstate).size());
}

TEST(OptionSelectorFilter, multi_selection)
{
    auto filter1 = OptionSelectorFilter::create("f1", "Options", true);
    auto option1 = filter1->add_option("1", "Option 1");
    auto option2 = filter1->add_option("2", "Option 2");
    filter1->add_option("3", "Option 3");

    FilterState fstate;

    // enable option1 & option2
    filter1->update_state(fstate, option1, true);
    filter1->update_state(fstate, option2, true);
    EXPECT_TRUE(fstate.has_filter("f1"));
    auto active = filter1->active_options(fstate);
    EXPECT_EQ(2u, active.size());
    EXPECT_TRUE(active.find(option1) != active.end());
    EXPECT_TRUE(active.find(option2) != active.end());

    // disable option1
    filter1->update_state(fstate, option1, false);
    EXPECT_EQ(1u, filter1->active_options(fstate).size());
    EXPECT_TRUE(active.find(option1) != active.end());

    // disable option2
    filter1->update_state(fstate, option2, false);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_EQ(0u, filter1->active_options(fstate).size());
}

TEST(OptionSelectorFilter, serialize)
{
    auto filter1 = OptionSelectorFilter::create("f1", "Options", true);
    filter1->set_display_hints(FilterBase::DisplayHints::Primary);
    filter1->add_option("1", "Option 1");
    filter1->add_option("2", "Option 2");

    auto var = filter1->serialize();
    EXPECT_EQ("f1", var["id"].get_string());
    EXPECT_EQ("option_selector", var["filter_type"].get_string());
    EXPECT_EQ("Options", var["label"].get_string());
    EXPECT_EQ(FilterBase::DisplayHints::Primary, static_cast<FilterBase::DisplayHints>(var["display_hints"].get_int()));

    auto optarr = var["options"].get_array();
    EXPECT_EQ(2u, optarr.size());
    EXPECT_EQ("1", optarr[0].get_dict()["id"].get_string());
    EXPECT_EQ("Option 1", optarr[0].get_dict()["label"].get_string());
    EXPECT_EQ("2", optarr[1].get_dict()["id"].get_string());
    EXPECT_EQ("Option 2", optarr[1].get_dict()["label"].get_string());
}

TEST(OptionSelectorFilter, deserialize)
{
    VariantMap var;
    {
        try
        {
            internal::OptionSelectorFilterImpl filter(var);
            FAIL();
        }
        catch (unity::InvalidArgumentException const&) {}
    }

    {
        var["id"] = "f1";
        var["filter_type"] = "option_selector";
        // missing attributes of option selector
        try
        {
            internal::OptionSelectorFilterImpl filter(var);
        }
        catch (unity::LogicException const&) {}
    }
    {
        var["id"] = "f1";
        var["filter_type"] = "option_selector";
        var["label"] = "Filter 1";
        var["multi_select"] = true;

        VariantArray optarr;
        VariantMap opt;
        opt["id"] = "1";
        opt["label"] = "Option 1";
        optarr.push_back(Variant(opt));
        var["options"] = optarr;
        internal::OptionSelectorFilterImpl filter(var);

        EXPECT_EQ("f1", filter.id());
        EXPECT_EQ("Filter 1", filter.label());
        EXPECT_EQ(true, filter.multi_select());
        EXPECT_EQ(1u, filter.options().size());
        EXPECT_EQ("1", filter.options().front()->id());
    }
}
