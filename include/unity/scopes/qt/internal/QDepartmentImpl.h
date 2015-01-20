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

    //    static QDepartment::UPtr create(QVariantMap const& var);
    //    static void validate_departments(QDepartment::SCPtr const& parent);
    //    static void validate_departments(QDepartment::SCPtr const& parent, QString const& current_id);
    //    static QVariantMap serialize_departments(QDepartment::SCPtr const& parent);
    //    static QDepartment::SCPtr find_subdepartment_by_id(QDepartment::SCPtr const& department, QString const& id);
    //
    // private:
    //    static void validate_departments(QDepartment::SCPtr const& department, std::unordered_set<QString>& lookup);

    std::shared_ptr<unity::scopes::Department> api_department_;

    friend class QDepartment;
};

}  // namespace qt

}  // namespace internal

}  // namespace scopes

}  // namespace unity
