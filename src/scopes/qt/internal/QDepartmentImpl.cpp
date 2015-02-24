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

#include <unity/scopes/qt/internal/QDepartmentImpl.h>
#include <unity/scopes/qt/internal/QCannedQueryImpl.h>
#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/scopes/Department.h>
#include <unity/scopes/CannedQuery.h>

#include <QtCore/QListIterator>

using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

namespace sc = unity::scopes;

QDepartmentImpl::QDepartmentImpl(QCannedQuery const& query, QString const& label)
    : api_department_(sc::Department::create(*query.p->api_query_, label.toUtf8().data()))
{
}

QDepartmentImpl::QDepartmentImpl(QString const& department_id, QCannedQuery const& query, QString const& label)
    : api_department_(
          sc::Department::create(department_id.toUtf8().data(), *query.p->api_query_, label.toUtf8().data()))
{
}

QDepartmentImpl::QDepartmentImpl(Department const& api_department)
    : api_department_(new sc::Department(api_department))
{
}

void QDepartmentImpl::set_has_subdepartments(bool subdepartments)
{
    api_department_->set_has_subdepartments(subdepartments);
}

void QDepartmentImpl::set_subdepartments(QDepartmentList const& departments)
{
    DepartmentList api_list;
    QListIterator<std::shared_ptr<QDepartment const>> it(departments);
    while (it.hasNext())
    {
        api_list.push_back(it.next().get()->p->api_department_);
    }
    api_department_->set_subdepartments(api_list);
}

void QDepartmentImpl::add_subdepartment(QDepartment::SCPtr const& department)
{
    api_department_->add_subdepartment(department->p->api_department_);
}

void QDepartmentImpl::set_alternate_label(QString const& label)
{
    api_department_->set_alternate_label(label.toUtf8().data());
}

QString QDepartmentImpl::alternate_label() const
{
    return QString::fromUtf8(api_department_->alternate_label().c_str());
}

QString QDepartmentImpl::id() const
{
    return QString::fromUtf8(api_department_->id().c_str());
}

QString QDepartmentImpl::label() const
{
    return QString::fromUtf8(api_department_->label().c_str());
}

QCannedQuery QDepartmentImpl::query() const
{
    return QCannedQuery(api_department_->query());
}

bool QDepartmentImpl::has_subdepartments() const
{
    return api_department_->has_subdepartments();
}

QDepartmentList QDepartmentImpl::subdepartments() const
{
    QDepartmentList ret_list;
    for (auto item : api_department_->subdepartments())
    {
        std::shared_ptr<QDepartment> qdepartment(new QDepartment(new QDepartmentImpl(*item)));
        ret_list.push_back(qdepartment);
    }
    return ret_list;
}

QVariantMap QDepartmentImpl::serialize() const
{
    return variantmap_to_qvariantmap(api_department_->serialize());
}

// added for testing purposes
unity::scopes::qt::QDepartment QDepartmentImpl::create(QDepartmentImpl* internal)
{
    return QDepartment(internal);
}
// added for testing purposes
unity::scopes::Department* QDepartmentImpl::get_api()
{
    return api_department_.get();
}
