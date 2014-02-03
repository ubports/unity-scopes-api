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

#include <unity/scopes/internal/DepartmentImpl.h>

namespace unity
{

namespace scopes
{

namespace internal
{
DepartmentImpl::DepartmentImpl(Query const& query, std::string const& label)
    : query_(query),
      label_(label)
{
}

DepartmentImpl::DepartmentImpl(std::string const& department_id, Query const& query, std::string const& label)
    : query_(query),
      label_(label)
{
    query_.set_department_id(department_id);
}

void DepartmentImpl::add_subdepartments(DepartmentList const& departments)
{
    departments_ = departments;
}

std::string DepartmentImpl::label() const
{
    return label_;
}

Query DepartmentImpl::query() const
{
    return query_;
}

DepartmentList DepartmentImpl::departments() const
{
    return departments_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
