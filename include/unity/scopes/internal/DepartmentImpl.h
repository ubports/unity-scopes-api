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
#include <unity/scopes/CannedQuery.h>
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
    DepartmentImpl(CannedQuery const& query, std::string const& label);
    DepartmentImpl(std::string const& department_id, CannedQuery const& query, std::string const& label);
    DepartmentImpl(std::string const& department_id, CannedQuery const& query, std::string const& label, DepartmentList const& subdepartments);
    DepartmentImpl(DepartmentImpl const&) = default;
    ~DepartmentImpl() = default;

    DepartmentImpl& operator=(DepartmentImpl const&) = default;

    void set_has_subdepartments();
    void set_subdepartments(DepartmentList const& departments);
    void set_alternate_label(std::string const& label);
    std::string alternate_label() const;

    std::string id() const;
    std::string label() const;
    CannedQuery query() const;
    bool has_subdepartments() const;
    DepartmentList subdepartments() const;
    VariantMap serialize() const;

    static Department::UPtr create(VariantMap const& var);
    static void validate_departments(Department::SCPtr const& parent, Department::SCPtr const& current);
    static VariantMap serialize_departments(Department::SCPtr const& parent, Department::SCPtr const& current);
    static DepartmentList deserialize_departments(VariantArray const& var);
    static Department::SCPtr find_subdepartment_by_id(Department::SCPtr const& department, std::string const& id);

private:
    static void validate_departments(Department::SCPtr const& department, std::unordered_set<std::string>& lookup);
    CannedQuery query_;
    std::string label_;
    std::string alt_label_;
    DepartmentList departments_;
    bool has_subdepartments_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
