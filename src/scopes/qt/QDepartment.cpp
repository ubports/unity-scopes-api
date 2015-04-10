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

#include <unity/scopes/qt/QDepartment.h>

#include <unity/scopes/qt/internal/QDepartmentImpl.h>

#include <unity/scopes/Department.h>

using namespace unity::scopes::qt;

/// @cond
QDepartment::QDepartment(internal::QDepartmentImpl* impl)
    : p(impl)
{
}

/// @cond
QDepartment::QDepartment(QDepartment const& other)
    : p(new internal::QDepartmentImpl(*(other.p)))
{
}

QDepartment::QDepartment(QDepartment&&) = default;

QDepartment::~QDepartment() = default;

QDepartment& QDepartment::operator=(QDepartment const& other)
{
    if (this != &other)
    {
        p.reset(new internal::QDepartmentImpl(*(other.p)));
    }
    return *this;
}

QDepartment& QDepartment::operator=(QDepartment&&) = default;

QDepartment::UPtr QDepartment::create(QCannedQuery const& query, QString const& label)
{
    return std::unique_ptr<QDepartment>(new QDepartment(new internal::QDepartmentImpl(query, label)));
}

QDepartment::UPtr QDepartment::create(QString const& department_id, QCannedQuery const& query, QString const& label)
{
    return std::unique_ptr<QDepartment>(new QDepartment(new internal::QDepartmentImpl(department_id, query, label)));
}

QVariantMap QDepartment::serialize() const
{
    return p->serialize();
}

/// @endcond

void QDepartment::set_has_subdepartments(bool subdepartments)
{
    p->set_has_subdepartments(subdepartments);
}

void QDepartment::set_subdepartments(QDepartmentList const& departments)
{
    p->set_subdepartments(departments);
}

void QDepartment::add_subdepartment(QDepartment::SCPtr const& department)
{
    p->add_subdepartment(department);
}

void QDepartment::set_alternate_label(QString const& label)
{
    p->set_alternate_label(label);
}

QString QDepartment::id() const
{
    return p->id();
}

QString QDepartment::label() const
{
    return p->label();
}

QString QDepartment::alternate_label() const
{
    return p->alternate_label();
}

QCannedQuery QDepartment::query() const
{
    return p->query();
}

bool QDepartment::has_subdepartments() const
{
    return p->has_subdepartments();
}

QDepartmentList QDepartment::subdepartments() const
{
    return p->subdepartments();
}

/// @endcond
