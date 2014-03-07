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
#include <unity/scopes/internal/CannedQueryImpl.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{
DepartmentImpl::DepartmentImpl(CannedQuery const& query, std::string const& label)
    : query_(query),
      label_(label)
{
}

DepartmentImpl::DepartmentImpl(std::string const& department_id, CannedQuery const& query, std::string const& label)
    : query_(query),
      label_(label)
{
    query_.set_department_id(department_id);
}

DepartmentImpl::DepartmentImpl(std::string const& department_id, CannedQuery const& query, std::string const& label, DepartmentList const& subdepartments)
    : query_(query),
      label_(label),
      departments_(subdepartments)
{
    query_.set_department_id(department_id);
}

void DepartmentImpl::set_subdepartments(DepartmentList const& departments)
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

CannedQuery DepartmentImpl::query() const
{
    return query_;
}

DepartmentList DepartmentImpl::subdepartments() const
{
    return departments_;
}

VariantMap DepartmentImpl::serialize() const
{
    VariantMap vm;
    vm["label"] = label_;
    vm["query"] = query_.serialize();

    // sub-departments are optional
    if (!departments_.empty())
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

Department DepartmentImpl::create(VariantMap const& var)
{
    auto it = var.find("label");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("DepartmentImpl::create(): missing 'label'");
    }
    auto label = it->second.get_string();
    it = var.find("query");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("DepartmentImpl::create(): missing 'query'");
    }
    auto query = CannedQueryImpl::create(it->second.get_dict());

    Department department(query, label);

    it = var.find("departments");
    if (it != var.end())
    {
        DepartmentList subdeps;
        for (auto const& dep: it->second.get_array())
        {
            subdeps.push_back(create(dep.get_dict()));
        }
        department.set_subdepartments(subdeps);
    }
    return department;
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
        validate_departments(dep.subdepartments(), lookup);
    }
}

void DepartmentImpl::validate_departments(DepartmentList const& departments, std::string const &current_department_id)
{
    if (departments.empty())
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

VariantMap DepartmentImpl::serialize_departments(DepartmentList const& departments, std::string const& current_department_id)
{
    VariantMap vm;
    VariantArray arr;
    for (auto const& dep: departments)
    {
        arr.push_back(Variant(dep.serialize()));
    }
    vm["departments"] = arr;
    vm["current_department"] = current_department_id;
    return vm;
}

} // namespace internal

} // namespace scopes

} // namespace unity
