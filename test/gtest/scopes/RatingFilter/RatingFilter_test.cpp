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
#include <unity/scopes/RatingFilter.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::experimental;
using namespace unity::scopes::internal;

TEST(RatingFilter, basic)
{
    {
        auto filter1 = RatingFilter::create("f1", "Rating");
        filter1->set_on_icon("foo");
        filter1->set_off_icon("bar");
        EXPECT_EQ("f1", filter1->id());
        EXPECT_EQ("Rating", filter1->label());
        EXPECT_EQ("foo", filter1->on_icon());
        EXPECT_EQ("bar", filter1->off_icon());
        EXPECT_EQ(0u, filter1->options().size());

        auto o1 = filter1->add_option("a", "A");
        auto o2 = filter1->add_option("b", "B");
        EXPECT_EQ("a", o1->id());
        EXPECT_EQ("b", o2->id());

        auto opts = filter1->options();
        EXPECT_EQ(2u, opts.size());
        EXPECT_EQ("a", opts.front()->id());
        EXPECT_EQ("A", opts.front()->label());
        EXPECT_EQ("b", opts.back()->id());
        EXPECT_EQ("B", opts.back()->label());
    }

    {
        auto filter1 = RatingFilter::create("f1", "Rating", 3);
        EXPECT_EQ("f1", filter1->id());
        EXPECT_EQ("Rating", filter1->label());

        auto opts = filter1->options();
        EXPECT_EQ(3u, opts.size());
        auto it = opts.begin();
        EXPECT_EQ("1", (*it)->id());
        EXPECT_EQ("1+", (*it)->label());
        EXPECT_EQ("2", (*++it)->id());
        EXPECT_EQ("2+", (*it)->label());
        EXPECT_EQ("3", (*++it)->id());
        EXPECT_EQ("3", (*it)->label());
    }

    {
        EXPECT_THROW(RatingFilter::create("f1", "Rating", 11), unity::LogicException);
    }

    {
        auto filter1 = RatingFilter::create("f1", "Rating", 10);
        EXPECT_THROW(filter1->add_option("11", "11"), unity::LogicException);
    }
}

TEST(RatingFilter, selection)
{
    auto filter1 = RatingFilter::create("f1", "Rating", 2);

    FilterState fstate;
    EXPECT_FALSE(fstate.has_filter("f1"));
    EXPECT_FALSE(filter1->has_active_rating(fstate));
    EXPECT_EQ(nullptr, filter1->active_rating(fstate));

    auto opts = filter1->options();
    EXPECT_EQ(2u, opts.size());

    auto it = opts.begin();
    auto option1 = *(it++);
    auto option2 = *it;

    // enable option1
    filter1->update_state(fstate, option1, true);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_TRUE(filter1->has_active_rating(fstate));
    auto active = filter1->active_rating(fstate);
    EXPECT_EQ("1", active->id());

    // enable option2, option1 get disabled
    filter1->update_state(fstate, option2, true);
    active = filter1->active_rating(fstate);
    EXPECT_EQ("2", active->id());

    // disable option1; filter state remains in the FilterState, just no options are selected
    filter1->update_state(fstate, option2, false);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_FALSE(filter1->has_active_rating(fstate));
    EXPECT_EQ(nullptr, filter1->active_rating(fstate));
}
