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

#ifndef UNITY_INTERNAL_DEPARTMENTIMPL_H
#define UNITY_INTERNAL_DEPARTMENTIMPL_H

#include <unity/scopes/Department.h>
#include <unity/scopes/Query.h>
#include <unordered_set>

namespace unity
{

namespace scopes
{

namespace internal
{

class DepartmentImpl
{
public:
    DepartmentImpl(Query const& query, std::string const& label);
    DepartmentImpl(std::string const& department_id, Query const& query, std::string const& label);
    DepartmentImpl(DepartmentImpl const&) = default;
    ~DepartmentImpl() = default;

    DepartmentImpl& operator=(DepartmentImpl const&) = default;

    void add_subdepartments(DepartmentList const& departments);

    std::string id() const;
    std::string label() const;
    Query query() const;
    DepartmentList departments() const;
    VariantMap serialize() const;

    static Department create(VariantMap const& var);
    static void validate_departments(DepartmentList const& departments, std::string const &current_department_id);

private:
    static void validate_departments(DepartmentList const& departments, std::unordered_set<std::string>& lookup);
    Query query_;
    std::string label_;
    DepartmentList departments_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
