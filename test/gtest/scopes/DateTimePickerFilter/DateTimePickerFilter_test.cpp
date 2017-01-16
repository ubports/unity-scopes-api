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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/FilterState.h>
#include <unity/scopes/DateTimePickerFilter.h>
#include <unity/scopes/internal/DateTimePickerFilterImpl.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/ScopeExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::experimental;
using namespace unity::scopes::internal;

TEST(DateTimePickerFilter, basic)
{
    DateTimePickerFilter::Mode mode(DateTimePickerFilter::Hours|DateTimePickerFilter::Minutes);
    auto filter = DateTimePickerFilter::create("f1", mode);
    EXPECT_EQ("f1", filter->id());
    EXPECT_EQ("", filter->time_label());
    EXPECT_EQ("", filter->date_label());
    EXPECT_EQ(filter->mode(), DateTimePickerFilter::Mode(DateTimePickerFilter::Hours|DateTimePickerFilter::Minutes));

    filter->set_time_label("foo");
    filter->set_date_label("bar");

    EXPECT_EQ("foo", filter->time_label());
    EXPECT_EQ("bar", filter->date_label());

    EXPECT_FALSE(filter->has_minimum());
    EXPECT_FALSE(filter->has_maximum());
    EXPECT_THROW(filter->minimum(), unity::LogicException);
    EXPECT_THROW(filter->maximum(), unity::LogicException);

    std::chrono::system_clock::time_point tp1 = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point tp2 = tp1 + std::chrono::seconds(10);

    filter->set_minimum(tp1);
    EXPECT_TRUE(filter->has_minimum());
    EXPECT_FALSE(filter->has_maximum());
    filter->set_maximum(tp2);
    EXPECT_TRUE(filter->has_minimum());
    EXPECT_TRUE(filter->has_maximum());

    EXPECT_EQ(tp1, filter->minimum());
    EXPECT_EQ(tp2, filter->maximum());
}

TEST(DateTimePickerFilter, timepoint_conversions)
{
    std::chrono::system_clock::time_point const tp = std::chrono::system_clock::now();
    int64_t t = internal::DateTimePickerFilterImpl::to_seconds_from_epoch(tp);
    EXPECT_TRUE(tp - internal::DateTimePickerFilterImpl::to_timepoint(t) < std::chrono::seconds(1));
}

TEST(DateTimePickerFilter, filter_state)
{
    FilterState state;
    DateTimePickerFilter::Mode mode(DateTimePickerFilter::Hours|DateTimePickerFilter::Minutes);
    auto filter = DateTimePickerFilter::create("f1", mode);

    EXPECT_FALSE(filter->has_selected_date(state));
    EXPECT_THROW(filter->selected_date(state), unity::scopes::NotFoundException);

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();

    filter->update_state(state, tp);
    EXPECT_TRUE(filter->has_selected_date(state));
    EXPECT_TRUE(tp - filter->selected_date(state) < std::chrono::seconds(1));

    tp += std::chrono::hours(2);
    filter->update_state(state, tp);
    EXPECT_TRUE(filter->has_selected_date(state));
    EXPECT_TRUE(tp - filter->selected_date(state) < std::chrono::seconds(1));

    // set minimum that is higher than selected date
    auto min = tp + std::chrono::hours(1);
    filter->set_minimum(min);
    EXPECT_THROW(filter->selected_date(state), unity::LogicException);

    // set maximum that is lower than selected date
    auto max = tp - std::chrono::hours(1);
    filter->set_minimum(tp);
    EXPECT_NO_THROW(filter->selected_date(state));
    filter->set_maximum(max);
    EXPECT_THROW(filter->selected_date(state), unity::LogicException);
}

TEST(DateTimePickerFilter, serialize)
{
    DateTimePickerFilter::Mode mode(DateTimePickerFilter::Hours|DateTimePickerFilter::Minutes);
    auto filter1 = DateTimePickerFilter::create("f1", mode);
    filter1->set_time_label("foo");
    filter1->set_date_label("bar");

    std::chrono::system_clock::time_point tp1 = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point tp2 = tp1 + std::chrono::seconds(10);
    filter1->set_minimum(tp1);
    filter1->set_maximum(tp2);

    auto var = filter1->serialize();
    auto filter2 = std::unique_ptr<internal::DateTimePickerFilterImpl>(new internal::DateTimePickerFilterImpl(var));

    EXPECT_EQ(filter1->id(), filter2->id());
    EXPECT_EQ(filter1->time_label(), filter2->time_label());
    EXPECT_EQ(filter1->date_label(), filter2->date_label());
    EXPECT_TRUE(filter1->minimum() - filter2->minimum() < std::chrono::seconds(1));
    EXPECT_TRUE(filter1->maximum() - filter2->maximum() < std::chrono::seconds(1));
    EXPECT_EQ(filter1->mode(), filter2->mode());
}

TEST(DateTimePickerFilter, display_hints)
{
    DateTimePickerFilter::Mode mode(DateTimePickerFilter::Hours|DateTimePickerFilter::Minutes);
    auto filter = DateTimePickerFilter::create("f1", mode);
    EXPECT_THROW(filter->set_display_hints(FilterBase::DisplayHints::Primary), unity::InvalidArgumentException);
}
