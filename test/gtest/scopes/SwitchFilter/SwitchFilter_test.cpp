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
#include <unity/scopes/SwitchFilter.h>
#include <unity/scopes/internal/SwitchFilterImpl.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(SwitchFilter, basic)
{
    auto filter1 = SwitchFilter::create("f1", "Latest");
    EXPECT_EQ("f1", filter1->id());
    EXPECT_EQ("Latest", filter1->label());

    FilterState fstate;
    EXPECT_FALSE(fstate.has_filter("f1"));
    EXPECT_FALSE(filter1->is_on(fstate));

    // switch on
    filter1->update_state(fstate, true);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_TRUE(filter1->is_on(fstate));

    // switch off
    filter1->update_state(fstate, false);
    EXPECT_TRUE(fstate.has_filter("f1"));
    EXPECT_FALSE(filter1->is_on(fstate));
}

TEST(SwitchFilter, serialize_deserialize)
{
    auto filter1 = SwitchFilter::create("f1", "Latest");
    auto var = filter1->serialize();
    auto filter2 = internal::SwitchFilterImpl::create(var);
    EXPECT_EQ("f1", filter2->id());
    EXPECT_EQ("Latest", filter2->label());
}

TEST(SwitchFilter, display_hints)
{
    auto filter = SwitchFilter::create("f1", "Latest");
    EXPECT_THROW(filter->set_display_hints(FilterBase::DisplayHints::Primary), unity::InvalidArgumentException);
}
