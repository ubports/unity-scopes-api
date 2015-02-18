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

#pragma once

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#include <unity/scopes/qt/QDepartment.h>
#include <unity/scopes/qt/QCannedQuery.h>

#include <QtCore/QString>

namespace unity
{

namespace scopes
{

class Department;

namespace qt
{

class QDepartment;

namespace internal
{

class QDepartmentImpl
{
public:
    QDepartmentImpl(QCannedQuery const& query, QString const& label);
    QDepartmentImpl(QString const& department_id, QCannedQuery const& query, QString const& label);
    QDepartmentImpl(Department const& api_query);
    QDepartmentImpl(QDepartmentImpl const&) = default;
    ~QDepartmentImpl() = default;

    QDepartmentImpl& operator=(QDepartmentImpl const&) = default;

    void set_has_subdepartments(bool subdepartments);
    void set_subdepartments(QDepartmentList const& departments);
    void add_subdepartment(QDepartment::SCPtr const& department);
    void set_alternate_label(QString const& label);
    QString alternate_label() const;

    QString id() const;
    QString label() const;
    QCannedQuery query() const;
    bool has_subdepartments() const;
    QDepartmentList subdepartments() const;
    QVariantMap serialize() const;

    std::shared_ptr<unity::scopes::Department> api_department_;

    static unity::scopes::qt::QDepartment create(QDepartmentImpl* internal);
    unity::scopes::Department* get_api();

    friend class QDepartment;
};

}  // namespace qt

}  // namespace internal

}  // namespace scopes

}  // namespace unity
