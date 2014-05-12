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
#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/internal/RangeInputFilterImpl.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RangeInputFilter, basic)
{
    auto filter1 = RangeInputFilter::create("f1", "Min", "Max", "EUR");
    EXPECT_EQ("f1", filter1->id());
    EXPECT_EQ("Min", filter1->start_label());
    EXPECT_EQ("Max", filter1->end_label());
    EXPECT_EQ("EUR", filter1->unit_label());
}

TEST(RangeInputFilter, state)
{
    auto filter1 = RangeInputFilter::create("f1", "Min", "Max", "EUR");

    FilterState fstate;
    EXPECT_FALSE(fstate.has_filter("f1"));

    filter1->update_state(fstate, Variant::null(), Variant::null());
    EXPECT_FALSE(fstate.has_filter("f1"));
    EXPECT_FALSE(filter1->has_start_value(fstate));
    EXPECT_FALSE(filter1->has_end_value(fstate));

    EXPECT_THROW(filter1->start_value(fstate), unity::LogicException);
    EXPECT_THROW(filter1->end_value(fstate), unity::LogicException);

    filter1->update_state(fstate, Variant(5), Variant::null());
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_TRUE(filter1->has_start_value(fstate));
    EXPECT_EQ(5.0f, filter1->start_value(fstate));
    EXPECT_FALSE(filter1->has_end_value(fstate));

    filter1->update_state(fstate, Variant(5), Variant(1.5f));
    EXPECT_TRUE(filter1->has_start_value(fstate));
    EXPECT_EQ(5.0f, filter1->start_value(fstate));
    EXPECT_TRUE(filter1->has_end_value(fstate));
    EXPECT_EQ(1.5f, filter1->end_value(fstate));
}

TEST(RangeInputFilter, serialize_deserialize)
{
    auto filter1 = RangeInputFilter::create("f1", "Min", "Max", "EUR");
    auto var = filter1->serialize();

    EXPECT_EQ("f1", var["id"].get_string());
    EXPECT_EQ("Min", var["start_label"].get_string());
    EXPECT_EQ("Max", var["end_label"].get_string());
    EXPECT_EQ("EUR", var["unit_label"].get_string());

    EXPECT_EQ("f1", var["id"].get_string());
    EXPECT_EQ("Min", var["start_label"].get_string());
    EXPECT_EQ("Max", var["end_label"].get_string());
    EXPECT_EQ("EUR", var["unit_label"].get_string());

    auto filter2 = internal::RangeInputFilterImpl::create(var);

    EXPECT_EQ("f1", filter2->id());
    EXPECT_EQ("Min", filter2->start_label());
    EXPECT_EQ("Max", filter2->end_label());
    EXPECT_EQ("EUR", filter2->unit_label());
}

TEST(RangeInputFilter, deserialize_exceptions)
{
    VariantMap var;
    var["id"] = "f1";
    EXPECT_THROW(internal::RangeInputFilterImpl::create(var), unity::InvalidArgumentException);
    var["start_label"] = "";
    EXPECT_THROW(internal::RangeInputFilterImpl::create(var), unity::InvalidArgumentException);
    var["end_label"] = "";
    EXPECT_THROW(internal::RangeInputFilterImpl::create(var), unity::InvalidArgumentException);
    var["unit_label"] = "";
    EXPECT_NO_THROW(internal::RangeInputFilterImpl::create(var));
}
