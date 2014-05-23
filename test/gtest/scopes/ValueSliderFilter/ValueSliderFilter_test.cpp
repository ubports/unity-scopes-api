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

#include <gtest/gtest.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/ValueSliderFilter.h>
#include <unity/scopes/internal/ValueSliderFilterImpl.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(ValueSliderFilter, basic)
{
    {
        auto filter1 = ValueSliderFilter::create("f1", "Max size", "Less than %d", 1, 100);
        EXPECT_EQ("f1", filter1->id());
        EXPECT_EQ("Max size", filter1->label());
        EXPECT_EQ("Less than %d", filter1->value_label_template());
        EXPECT_EQ(1, filter1->min());
        EXPECT_EQ(100, filter1->max());
        EXPECT_EQ(ValueSliderFilter::SliderType::LessThan, filter1->slider_type());
        filter1->set_slider_type(ValueSliderFilter::SliderType::MoreThan);
        EXPECT_EQ(ValueSliderFilter::SliderType::MoreThan, filter1->slider_type());
    }

    {
        EXPECT_THROW(ValueSliderFilter::create("f1", "Max size", "Less than %d", 10, 1), unity::LogicException);
        EXPECT_THROW(ValueSliderFilter::create("f1", "Max size", "Less than %d", -1, 10), unity::LogicException);
    }
}

TEST(ValueSliderFilter, state)
{
    {
        FilterState fstate;
        auto filter1 = ValueSliderFilter::create("f1", "Max size", "Less than %d", 1, 100);
        EXPECT_EQ("Less than 100", filter1->value_label(fstate));
        EXPECT_FALSE(filter1->has_value(fstate));
        EXPECT_THROW(filter1->value(fstate), unity::LogicException);
    }

    {
        FilterState fstate;
        auto filter1 = ValueSliderFilter::create("f1", "Max size", "Less than", 1, 100);
        EXPECT_THROW(filter1->value_label(fstate), unity::LogicException); // invalid label template
    }

    {
        FilterState fstate;
        auto filter1 = ValueSliderFilter::create("f1", "Max size", "Less than %d", 1, 100);
        filter1->update_state(fstate, 33);
        EXPECT_EQ("Less than 33", filter1->value_label(fstate));
        EXPECT_TRUE(filter1->has_value(fstate));
        EXPECT_EQ(33, filter1->value(fstate));
        EXPECT_THROW(filter1->update_state(fstate, 0), unity::LogicException);
        EXPECT_THROW(filter1->update_state(fstate, 999), unity::LogicException);
    }
}

TEST(ValueSliderFilter, serialize)
{
    auto filter1 = ValueSliderFilter::create("f1", "Max size", "Less than %d", 1, 100);
    auto var = filter1->serialize();

    auto filter2 = internal::ValueSliderFilterImpl::create(var);
    EXPECT_EQ("f1", filter1->id());
    EXPECT_EQ("Max size", filter1->label());
    EXPECT_EQ("Less than %d", filter1->value_label_template());
    EXPECT_EQ(1, filter1->min());
    EXPECT_EQ(100, filter1->max());
    EXPECT_EQ(ValueSliderFilter::SliderType::LessThan, filter1->slider_type());
}
