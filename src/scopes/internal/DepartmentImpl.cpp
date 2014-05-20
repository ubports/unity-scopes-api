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
#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{
DepartmentImpl::DepartmentImpl(CannedQuery const& query, std::string const& label)
    : query_(query),
      label_(label),
      has_subdepartments_(false)
{
    if (label.empty())
    {
        throw InvalidArgumentException("Department(): Invalid empty label string");
    }
}

DepartmentImpl::DepartmentImpl(std::string const& department_id, CannedQuery const& query, std::string const& label)
    : query_(query),
      label_(label),
      has_subdepartments_(false)
{
    if (label.empty())
    {
        throw InvalidArgumentException("Department(): Invalid empty label string");
    }
    query_.set_department_id(department_id);
}

void DepartmentImpl::set_has_subdepartments()
{
    has_subdepartments_ = true;
}

void DepartmentImpl::set_subdepartments(DepartmentList const& departments)
{
    for (auto const& dep: departments)
    {
        if (dep == nullptr)
        {
            std::stringstream str;
            str << "DepartmentImpl::set_subdepartments(): invalid null department, parent department '", id();
            throw InvalidArgumentException(str.str());
        }
    }
    departments_ = departments;
}

std::string DepartmentImpl::id() const
{
    return query_.department_id();
}

void DepartmentImpl::set_alternate_label(std::string const& label)
{
    alt_label_ = label;
}

std::string DepartmentImpl::alternate_label() const
{
    return alt_label_;
}

std::string DepartmentImpl::label() const
{
    return label_;
}

CannedQuery DepartmentImpl::query() const
{
    return query_;
}

bool DepartmentImpl::has_subdepartments() const
{
    return departments_.size() > 0 || has_subdepartments_;
}

DepartmentList DepartmentImpl::subdepartments() const
{
    return departments_;
}

VariantMap DepartmentImpl::serialize() const
{
    VariantMap vm;
    vm["label"] = label_;
    if (alt_label_.size())
    {
        vm["alt_label"] = alt_label_;
    }
    vm["query"] = query_.serialize();

    // sub-departments are optional
    if (departments_.empty())
    {
        if (has_subdepartments_)
        {
            vm["has_subdepartments"] = Variant(has_subdepartments_);
        }
    }
    else
    {
        VariantArray subdeparr;
        for (auto const& dep: departments_)
        {
            subdeparr.push_back(Variant(dep->serialize()));
        }

        vm["departments"] = Variant(subdeparr);
    }
    return vm;
}

Department::UPtr DepartmentImpl::create(VariantMap const& var)
{
    auto it = var.find("label");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("DepartmentImpl::create(): missing 'label'");
    }
    auto label = it->second.get_string();
    std::string alt_label;
    it = var.find("alt_label");
    if (it != var.end())
    {
        alt_label = it->second.get_string();
    }
    it = var.find("query");
    if (it == var.end())
    {
        throw unity::InvalidArgumentException("DepartmentImpl::create(): missing 'query'");
    }
    auto query = CannedQueryImpl::create(it->second.get_dict());

    Department::UPtr department = Department::create(query, label);
    department->set_alternate_label(alt_label);

    it = var.find("departments");
    if (it != var.end())
    {
        DepartmentList subdeps;
        for (auto const& dep: it->second.get_array())
        {
            subdeps.push_back(create(dep.get_dict()));
        }
        department->set_subdepartments(subdeps);
    }

    it = var.find("has_subdepartments");
    if (it != var.end())
    {
        if (it->second.get_bool())
        {
            department->set_has_subdepartments();
        }
    }
    return department;
}

void DepartmentImpl::validate_departments(Department::SCPtr const& department, std::unordered_set<std::string>& lookup)
{
    if (lookup.find(department->id()) != lookup.end())
    {
        std::stringstream str;
        str << "DepartmentImpl::validate_departments(): duplicate department: '" << department->id() << "'";
        throw unity::LogicException(str.str());
    }

    lookup.insert(department->id());
    for (auto const& dep: department->p->departments_)
    {
        validate_departments(dep, lookup);
    }
}

void DepartmentImpl::validate_departments(Department::SCPtr const& parent, Department::SCPtr const& current)
{
    if (parent == nullptr)
    {
        throw unity::LogicException("DepartmentImpl::validate_departments(): invalid null parent department");
    }

    if (current == nullptr)
    {
        throw unity::LogicException("DepartmentImpl::validate_departments(): invalid null current department");
    }

    if (parent->subdepartments().size() == 0)
    {
        std::stringstream str;
        str << "DepartmentImpl::validate_departments(): at least two levels of departments required, parent department id '"
            << parent->id() << "'";
        throw unity::LogicException(str.str());
    }

    // don't allow duplicated department ids.
    std::unordered_set<std::string> lookup;
    validate_departments(parent, lookup);
    if (parent != current)
    {
        // ensure that current department matches one of the (sub)departments
        if (lookup.find(current->id()) == lookup.end())
        {
            std::stringstream str;
            str << "DepartmentImpl::validate_departments(): current department '" << current->id() <<
                "' doesn't match any of the subdepartments of parent '" << parent->id() << "'";
            throw unity::LogicException(str.str());
        }
    }
}

Department::SCPtr DepartmentImpl::find_subdepartment_by_id(Department::SCPtr const& department, std::string const& id)
{
    assert(department);

    if (department->id() == id)
    {
        return department;
    }

    for (auto const& dep: department->p->departments_)
    {
        auto res = find_subdepartment_by_id(dep, id);
        if (res != nullptr)
        {
            return res;
        }
    }
    return nullptr;
}

VariantMap DepartmentImpl::serialize_departments(Department::SCPtr const& parent, Department::SCPtr const& current)
{
    VariantMap vm;
    vm["departments"] = Variant(parent->serialize());
    vm["current_department"] = current->id();
    return vm;
}

} // namespace internal

} // namespace scopes

} // namespace unity
