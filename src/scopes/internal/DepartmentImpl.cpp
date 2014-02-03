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
#include <unity/UnityExceptions.h>
#include <sstream>

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

std::string DepartmentImpl::id() const
{
    return query_.department_id();
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

VariantMap DepartmentImpl::serialize() const
{
    VariantMap vm;
    vm["label"] = label_;
    vm["query"] = query_.serialize();

    // sub-departments are optional
    if (departments_.size())
    {
        VariantArray subdeparr;
        for (auto const& dep: departments_)
        {
            subdeparr.push_back(Variant(dep.serialize()));
        }

        vm["departments"] = Variant(subdeparr);
    }
    return vm;
}

void DepartmentImpl::validate_departments(DepartmentList const& departments, std::unordered_set<std::string>& lookup)
{
    for (auto const& dep: departments)
    {
        if (lookup.find(dep.id()) != lookup.end())
        {
            std::stringstream str;
            str << "DepartmentImpl::validate_departments(): Duplicate department: '" << dep.id() << "'";
            throw unity::LogicException(str.str());
        }
        lookup.insert(dep.id());
        validate_departments(dep.departments(), lookup);
    }
}

void DepartmentImpl::validate_departments(DepartmentList const& departments, std::string const &current_department_id)
{
    if (departments.size() == 0)
    {
        throw unity::LogicException("DepartmentImpl::validate_departments(): empty departments list");
    }

    // don't allow duplicated department ids. ensure at current_department_id matches one of the departments (if non-empty).
    std::unordered_set<std::string> lookup;
    validate_departments(departments, lookup);
    if (!current_department_id.empty())
    {
        if (lookup.find(current_department_id) == lookup.end())
        {
            std::stringstream str;
            str << "DepartmentImpl::validate_departments(): current department '" << current_department_id << "' doesn't match any of the departments";
            throw unity::LogicException(str.str());
        }
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
