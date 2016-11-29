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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/qt/QCannedQuery.h>
#include <unity/scopes/qt/QDepartment.h>
#include <unity/scopes/qt/internal/QDepartmentImpl.h>
#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/scopes/Department.h>

#include <iostream>

using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

TEST(QDepartment, bindings)
{
    QCannedQuery query("scopeA", "foo", "dep1");
    QCannedQuery query2("scopeA", "foo2", "dep2");
    QCannedQuery query3("scopeA", "foo3", "dep3");

    internal::QDepartmentImpl* impl = new internal::QDepartmentImpl(query, "label");
    QDepartment dep = internal::QDepartmentImpl::create(impl);
    EXPECT_EQ("label", dep.label());
    EXPECT_EQ("foo", dep.query().query_string());
    EXPECT_FALSE(dep.has_subdepartments());
    dep.set_has_subdepartments(false);
    EXPECT_FALSE(dep.has_subdepartments());
    dep.set_has_subdepartments(true);
    EXPECT_TRUE(dep.has_subdepartments());

    // get the internal api layout
    unity::scopes::Department* api_dep = impl->get_api();

    dep.set_alternate_label("alternate_label");
    EXPECT_EQ(api_dep->alternate_label(), dep.alternate_label().toStdString());
    EXPECT_EQ(api_dep->id(), dep.id().toStdString());

    // add subdepartments
    std::shared_ptr<QDepartment const> dep2(QDepartment::create(query2, "label2"));
    std::shared_ptr<QDepartment const> dep3(QDepartment::create(query2, "label3"));
    dep.add_subdepartment(dep2);
    dep.add_subdepartment(dep3);

    QDepartmentList qt_list = dep.subdepartments();
    unity::scopes::DepartmentList api_list = api_dep->subdepartments();

    EXPECT_TRUE(qt_list.size() != 0);
    EXPECT_EQ(qt_list.size(), int(api_list.size()));

    std::vector<std::string> dep_ids;
    for (auto item : api_list)
    {
        dep_ids.push_back(item->id());
    }

    std::vector<std::string> qt_dep_ids;
    QListIterator<std::shared_ptr<QDepartment const>> it(qt_list);
    while (it.hasNext())
    {
        qt_dep_ids.push_back(it.next()->id().toStdString());
    }

    for (uint i = 0; i < qt_dep_ids.size(); ++i)
    {
        EXPECT_EQ(qt_dep_ids[i], dep_ids[i]);
    }

    EXPECT_EQ(variantmap_to_qvariantmap(api_dep->serialize()), dep.serialize());

    // TODO: need coverage for set_subdepartments() once we work out the correct
    // typedef for QDepartmentList.

    std::shared_ptr<QDepartment const> dep4(QDepartment::create(query2, "label4"));
    std::shared_ptr<QDepartment const> dep5(QDepartment::create(query2, "label5"));
    QDepartmentList l;
    l.append(dep4);
    l.append(dep5);
    dep.set_subdepartments(l);
    auto l2 = dep.subdepartments();
    ASSERT_EQ(2, l2.size());
    it = l2;
    auto d = it.next();
    EXPECT_EQ("label4", d->label());
    d = it.next();
    EXPECT_EQ("label5", d->label());
}

TEST(QDepartment, construct_assign)
{
    QCannedQuery query("scopeA", "foo", "dep1");
    internal::QDepartmentImpl* impl = new internal::QDepartmentImpl(query, "label");
    QDepartment dep = internal::QDepartmentImpl::create(impl);

    QDepartment dep2(dep);
    EXPECT_EQ("label", dep2.label());

    internal::QDepartmentImpl* impl3 = new internal::QDepartmentImpl(query, "label3");
    QDepartment dep3 = internal::QDepartmentImpl::create(impl3);
    EXPECT_EQ("label3", dep3.label());

    dep3 = dep2;
    EXPECT_EQ("label", dep3.label());

    auto dep4 = QDepartment::create("id", query, "l");
    EXPECT_EQ("id", dep4->id());
    EXPECT_EQ("l", dep4->label());
}
