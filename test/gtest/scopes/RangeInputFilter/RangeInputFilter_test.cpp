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

#include <cmath>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/internal/RangeInputFilterImpl.h>
#include <unity/scopes/ScopeExceptions.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(RangeInputFilter, basic)
{
    {
        auto filter1 = RangeInputFilter::create("f1", "Min", "MinPostfix", "Central", "Max", "MaxPostfix");
        EXPECT_EQ("f1", filter1->id());
        EXPECT_EQ("Min", filter1->start_prefix_label());
        EXPECT_EQ("MinPostfix", filter1->start_postfix_label());
        EXPECT_EQ("Max", filter1->end_prefix_label());
        EXPECT_EQ("MaxPostfix", filter1->end_postfix_label());
        EXPECT_EQ("Central", filter1->central_label());
    }
    {
        auto filter1 = RangeInputFilter::create("f1", Variant(10), Variant(20), "Min", "MinPostfix", "Central", "Max", "MaxPostfix");
        EXPECT_EQ("f1", filter1->id());
        EXPECT_EQ(10, filter1->default_start_value().get_int());
        EXPECT_EQ(20, filter1->default_end_value().get_int());
        EXPECT_EQ("Min", filter1->start_prefix_label());
        EXPECT_EQ("MinPostfix", filter1->start_postfix_label());
        EXPECT_EQ("Max", filter1->end_prefix_label());
        EXPECT_EQ("MaxPostfix", filter1->end_postfix_label());
        EXPECT_EQ("Central", filter1->central_label());
    }
}

TEST(RangeInputFilter, state)
{
    {
        auto filter1 = RangeInputFilter::create("f1", Variant(10.0f), Variant(20.0f), "", "", "", "", "");

        FilterState fstate;
        EXPECT_FALSE(fstate.has_filter("f1"));
        EXPECT_FALSE(filter1->has_start_value(fstate));
        EXPECT_FALSE(filter1->has_end_value(fstate));

        EXPECT_TRUE(std::abs(filter1->start_value(fstate) - 10.0f) < 0.0001f);
        EXPECT_TRUE(std::abs(filter1->end_value(fstate) - 20.0f) < 0.0001f);
    }

    {
        auto filter1 = RangeInputFilter::create("f1", Variant(5), Variant(10), "", "", "", "", "");

        FilterState fstate;
        EXPECT_FALSE(fstate.has_filter("f1"));
        EXPECT_FALSE(filter1->has_start_value(fstate));
        EXPECT_FALSE(filter1->has_end_value(fstate));

        EXPECT_TRUE(std::abs(filter1->start_value(fstate) - 5) < 0.0001f);
        EXPECT_TRUE(std::abs(filter1->end_value(fstate) - 10) < 0.0001f);
    }

    {
        auto filter1 = RangeInputFilter::create("f1", Variant::null(), Variant::null(), "", "", "", "", "");

        FilterState fstate;
        EXPECT_FALSE(fstate.has_filter("f1"));
        EXPECT_FALSE(filter1->has_start_value(fstate));
        EXPECT_FALSE(filter1->has_end_value(fstate));

        EXPECT_THROW(filter1->start_value(fstate), NotFoundException);
        EXPECT_THROW(filter1->end_value(fstate), NotFoundException);
    }

    {
        auto filter1 = RangeInputFilter::create("f1", "", "", "", "", "");

        FilterState fstate;
        EXPECT_FALSE(fstate.has_filter("f1"));

        filter1->update_state(fstate, Variant::null(), Variant::null());
        EXPECT_FALSE(fstate.has_filter("f1"));
        EXPECT_FALSE(filter1->has_start_value(fstate));
        EXPECT_FALSE(filter1->has_end_value(fstate));

        EXPECT_THROW(filter1->start_value(fstate), unity::scopes::NotFoundException);
        EXPECT_THROW(filter1->end_value(fstate), unity::scopes::NotFoundException);

        filter1->update_state(fstate, Variant(5), Variant::null());
        EXPECT_TRUE(fstate.has_filter("f1"));
        EXPECT_TRUE(filter1->has_start_value(fstate));
        EXPECT_EQ(5.0f, filter1->start_value(fstate));
        EXPECT_FALSE(filter1->has_end_value(fstate));

        filter1->update_state(fstate, Variant(5), Variant(6.5f));
        EXPECT_TRUE(filter1->has_start_value(fstate));
        EXPECT_EQ(5.0f, filter1->start_value(fstate));
        EXPECT_TRUE(filter1->has_end_value(fstate));
        EXPECT_EQ(6.5f, filter1->end_value(fstate));

        EXPECT_THROW(filter1->update_state(fstate, Variant(5), Variant(0.5f)), unity::LogicException);
    }
}

TEST(RangeInputFilter, serialize_deserialize)
{
    auto filter1 = RangeInputFilter::create("f1", Variant(10), Variant(20), "Min", "MinPostfix", "Central", "Max", "MaxPostfix");
    auto var = filter1->serialize();

    auto filter2 = internal::RangeInputFilterImpl::create(var);

    EXPECT_EQ("f1", filter2->id());
    EXPECT_EQ(10, filter1->default_start_value().get_int());
    EXPECT_EQ(20, filter1->default_end_value().get_int());
    EXPECT_EQ("Min", filter2->start_prefix_label());
    EXPECT_EQ("MinPostfix", filter2->start_postfix_label());
    EXPECT_EQ("Max", filter2->end_prefix_label());
    EXPECT_EQ("MaxPostfix", filter2->end_postfix_label());
    EXPECT_EQ("Central", filter2->central_label());
}

TEST(RangeInputFilter, deserialize_exceptions)
{
    std::vector<std::string> attrs = {"id", "start_prefix_label", "start_postfix_label", "end_prefix_label", "end_postfix_label", "central_label",
        "default_start_value", "default_end_value"};
    for (size_t i = 0; i<attrs.size(); i++)
    {
        // create variant with one of the mandatory attributes missing
        VariantMap var;
        for (size_t j = 0; j<attrs.size(); j++)
        {
            if (i != j)
            {
                var[attrs[i]] = "foo";
            }
        }
        EXPECT_THROW(RangeInputFilterImpl::create(var), unity::scopes::NotFoundException);
    }

    VariantMap var;
    for (size_t i = 0; i<attrs.size(); i++)
    {
        var[attrs[i]] = "foo";
    }
    EXPECT_NO_THROW(RangeInputFilterImpl::create(var));
}

TEST(RangeInputFilter, display_hints)
{
    auto filter = RangeInputFilter::create("f1", "", "", "", "", "");
    EXPECT_THROW(filter->set_display_hints(FilterBase::DisplayHints::Primary), unity::InvalidArgumentException);
}
