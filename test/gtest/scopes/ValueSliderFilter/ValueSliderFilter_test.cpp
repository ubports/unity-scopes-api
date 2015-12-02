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
#include <unity/scopes/ValueSliderLabels.h>
#include <unity/scopes/internal/ValueSliderFilterImpl.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::experimental;
using namespace unity::scopes::internal;

TEST(ValueSliderFilter, basic)
{
    {
        auto filter1 = ValueSliderFilter::create("f1", 1, 100, 100, ValueSliderLabels("Min", "Max"));
        EXPECT_EQ("f1", filter1->id());
        EXPECT_EQ("Min", filter1->labels().min_label());
        EXPECT_EQ("Max", filter1->labels().max_label());
        EXPECT_EQ(100, filter1->default_value());
        EXPECT_EQ(1, filter1->min());
        EXPECT_EQ(100, filter1->max());
        filter1->set_default_value(2);
        EXPECT_EQ(2, filter1->default_value());
    }
    {
        auto filter1 = ValueSliderFilter::create("f1", 1, 100, 50, ValueSliderLabels("Min", "Max", {{10, "Ten"}, {50, "Fifty"}}));
        EXPECT_EQ("f1", filter1->id());
        EXPECT_EQ("Min", filter1->labels().min_label());
        EXPECT_EQ("Max", filter1->labels().max_label());
        EXPECT_EQ(2, filter1->labels().extra_labels().size());
        EXPECT_EQ("Ten", filter1->labels().extra_labels()[0].second);
        EXPECT_EQ("Fifty", filter1->labels().extra_labels()[1].second);
        EXPECT_EQ(50, filter1->default_value());
        EXPECT_EQ(1, filter1->min());
        EXPECT_EQ(100, filter1->max());
    }
    {
        EXPECT_THROW(ValueSliderFilter::create("f1", 10, 1, 1, ValueSliderLabels("Min", "Max")), unity::LogicException);
        EXPECT_THROW(ValueSliderFilter::create("f1", -1, 10, 1, ValueSliderLabels("Min", "Max")), unity::LogicException);
        EXPECT_THROW(ValueSliderFilter::create("f1", 1, 10, 20, ValueSliderLabels("Min", "Max")), unity::LogicException);
    }
}

TEST(ValueSliderFilter, state)
{
    {
        FilterState fstate;
        auto filter1 = ValueSliderFilter::create("f1", 1, 100, 100, ValueSliderLabels("Min", "Max"));
        EXPECT_FALSE(filter1->has_value(fstate));
        EXPECT_THROW(filter1->value(fstate), unity::scopes::NotFoundException);
    }

    {
        FilterState fstate;
        auto filter1 = ValueSliderFilter::create("f1", 1, 100, 100, ValueSliderLabels("Min", "Max"));
        filter1->update_state(fstate, 33);
        EXPECT_TRUE(filter1->has_value(fstate));
        EXPECT_EQ(33, filter1->value(fstate));
        EXPECT_THROW(filter1->update_state(fstate, 0), unity::LogicException);
        EXPECT_THROW(filter1->update_state(fstate, 999), unity::LogicException);
    }
}

TEST(ValueSliderFilter, serialize)
{
    auto filter1 = ValueSliderFilter::create("f1", 1, 100, 100, ValueSliderLabels("Min", "Max"));
    auto var = filter1->serialize();

    auto filter2 = internal::ValueSliderFilterImpl::create(var);
    EXPECT_EQ("f1", filter1->id());
    EXPECT_EQ("Min", filter1->labels().min_label());
    EXPECT_EQ("Max", filter1->labels().max_label());
    EXPECT_EQ(1, filter1->min());
    EXPECT_EQ(100, filter1->max());
}

TEST(ValueSliderFilter, display_hints)
{
    auto filter = ValueSliderFilter::create("f1", 1, 100, 1, ValueSliderLabels("Min", "Max"));
    EXPECT_THROW(filter->set_display_hints(FilterBase::DisplayHints::Primary), unity::InvalidArgumentException);
}
