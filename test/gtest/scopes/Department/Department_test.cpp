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

#include <unity/scopes/Department.h>
#include <unity/scopes/internal/DepartmentImpl.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/UnityExceptions.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Department, basic)
{
    {
        CannedQuery query("fooscope", "foo", "dep1");
        Department::SPtr dep = Department::create(query, "News");
        dep->set_alternate_label("All News");

        EXPECT_EQ("dep1", dep->id());
        EXPECT_EQ("dep1", dep->query().department_id());
        EXPECT_EQ("fooscope", dep->query().scope_id());
        EXPECT_EQ("News", dep->label());
        EXPECT_EQ("All News", dep->alternate_label());
        EXPECT_FALSE(dep->has_subdepartments());

        dep->set_subdepartments({Department::create("subdep1", query, "Europe")});
        EXPECT_TRUE(dep->has_subdepartments());
        EXPECT_EQ(1u, dep->subdepartments().size());
        EXPECT_EQ("subdep1", dep->subdepartments().front()->id());
        EXPECT_EQ("subdep1", dep->subdepartments().front()->query().department_id());
        EXPECT_EQ("Europe", dep->subdepartments().front()->label());

        dep->add_subdepartment(std::move(Department::create("subdep2", query, "Australia")));
        EXPECT_EQ(2u, dep->subdepartments().size());
        EXPECT_EQ("subdep2", dep->subdepartments().back()->id());
    }
    {
        CannedQuery query("fooscope", "foo", "dep1");
        Department::SPtr dep = Department::create(query, "News");
        EXPECT_FALSE(dep->has_subdepartments());

        dep->set_has_subdepartments();
        EXPECT_TRUE(dep->has_subdepartments());
    }

    {
        CannedQuery query("fooscope", "foo", "dep1");
        Department::SPtr dep = Department::create(query, "News");
        dep->set_subdepartments({Department::create("subdep1", query, "Europe")});
        EXPECT_THROW(dep->set_has_subdepartments(false), unity::LogicException);
    }

    {
        CannedQuery query("fooscope", "foo", "dep1");
        Department::SPtr dep = Department::create(query, "News");
        dep->set_subdepartments({Department::create("subdep1", query, "Europe")});
        dep->set_subdepartments({});
        EXPECT_NO_THROW(dep->set_has_subdepartments(false));
    }
}

TEST(Department, serialize_and_deserialize)
{
    VariantMap var;
    {
        CannedQuery query("fooscope", "foo", "dep1");
        Department::SPtr dep = Department::create(query, "News");
        dep->set_alternate_label("All News");
        dep->set_subdepartments({Department::create("subdep1", query, "Europe"), Department::create("subdep2", query, "US")});

        var = dep->serialize();
    }

    // deserialize back
    auto dep2 = internal::DepartmentImpl::create(var);
    EXPECT_EQ("dep1", dep2->id());
    EXPECT_EQ("dep1", dep2->query().department_id());
    EXPECT_EQ("fooscope", dep2->query().scope_id());
    EXPECT_EQ("News", dep2->label());
    EXPECT_EQ("All News", dep2->alternate_label());
    EXPECT_EQ(2u, dep2->subdepartments().size());
    EXPECT_EQ("subdep1", dep2->subdepartments().front()->id());
    EXPECT_EQ("subdep1", dep2->subdepartments().front()->query().department_id());
    EXPECT_EQ("Europe", dep2->subdepartments().front()->label());
}
