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
#include <gtest/gtest.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Department, basic)
{
    CannedQuery query("fooscope", "foo", "dep1");
    Department dep(query, "News");

    EXPECT_EQ("dep1", dep.id());
    EXPECT_EQ("dep1", dep.query().department_id());
    EXPECT_EQ("fooscope", dep.query().scope_id());
    EXPECT_EQ("News", dep.label());

    dep.set_subdepartments({{"subdep1", query, "Europe"}});
    EXPECT_EQ(1u, dep.subdepartments().size());
    EXPECT_EQ("subdep1", dep.subdepartments().front().id());
    EXPECT_EQ("subdep1", dep.subdepartments().front().query().department_id());
    EXPECT_EQ("Europe", dep.subdepartments().front().label());
}

TEST(Department, serialize_and_deserialize)
{
    VariantMap var;
    {
        CannedQuery query("fooscope", "foo", "dep1");
        Department dep(query, "News");
        dep.set_subdepartments({{"subdep1", query, "Europe"},{"subdep2", query, "US"}});

        var = dep.serialize();
    }

    // deserialize back
    auto dep2 = internal::DepartmentImpl::create(var);
    EXPECT_EQ("dep1", dep2.id());
    EXPECT_EQ("dep1", dep2.query().department_id());
    EXPECT_EQ("fooscope", dep2.query().scope_id());
    EXPECT_EQ("News", dep2.label());
    EXPECT_EQ(2u, dep2.subdepartments().size());
    EXPECT_EQ("subdep1", dep2.subdepartments().front().id());
    EXPECT_EQ("subdep1", dep2.subdepartments().front().query().department_id());
    EXPECT_EQ("Europe", dep2.subdepartments().front().label());
}
