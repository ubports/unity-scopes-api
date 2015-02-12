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

#include <gtest/gtest.h>

#include <unity/scopes/qt/QCannedQuery.h>
#include <unity/scopes/qt/QDepartment.h>
#include <unity/scopes/qt/QUtils.h>
#include <unity/scopes/qt/internal/QDepartmentImpl.h>

#include <unity/scopes/Department.h>

#include <iostream>

using namespace unity::scopes::qt;

TEST(QDepartment, bindings)
{
    QCannedQuery query("scopeA", "foo", "dep1");
    QCannedQuery query2("scopeA", "foo2", "dep2");
    QCannedQuery query3("scopeA", "foo3", "dep3");

    internal::QDepartmentImpl *impl = new internal::QDepartmentImpl(query, "label");
    QDepartment dep = internal::QDepartmentImpl::create(impl);

    // get the internal api layout
    unity::scopes::Department * api_dep = impl->get_api();

    dep.set_alternate_label("alternate_label");
    EXPECT_EQ(api_dep->alternate_label(), dep.alternate_label().toStdString());
    EXPECT_EQ(api_dep->id(), dep.id().toStdString());

    // add subdepartments
    std::shared_ptr<QDepartment const> dep2(QDepartment::create(query2, "label2"));
    std::shared_ptr<QDepartment const> dep3(QDepartment::create(query2, "label2"));
    dep.add_subdepartment(dep2);
    dep.add_subdepartment(dep3);

    QDepartmentList qt_list = dep.subdepartments();
    unity::scopes::DepartmentList api_list = api_dep->subdepartments();

    EXPECT_TRUE(qt_list.size() != 0);
    EXPECT_EQ(qt_list.size(), api_list.size());

    std::vector<std::string> dep_ids;
    for(auto item: api_list)
    {
        dep_ids.push_back(item->id());
    }

    std::vector<std::string> qt_dep_ids;
    QListIterator<QSharedPointer<QDepartment const>> it(qt_list);
    while(it.hasNext())
    {
        qt_dep_ids.push_back(it.next()->id().toStdString());
    }

    for(uint i=0; i < qt_dep_ids.size(); ++i)
    {
        EXPECT_EQ(qt_dep_ids[i], dep_ids[i]);
    }

    EXPECT_EQ(variantmap_to_qvariantmap(api_dep->serialize()), dep.serialize());
}
