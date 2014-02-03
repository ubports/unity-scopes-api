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

#ifndef UNITY_SCOPES_DEPARTMENT_H
#define UNITY_SCOPES_DEPARTMENT_H

#include <unity/SymbolExport.h>
#include <unity/scopes/Variant.h>
#include <list>
#include <string>
#include <memory>

namespace unity
{

namespace scopes
{

class Query;
class Department;
typedef std::list<Department> DepartmentList;

namespace internal
{
class DepartmentImpl;
}

/**
\brief Definition of a department and its optional sub-departments
*/
class UNITY_API Department final
{
public:
    /**
    \brief Create deparment with given search Query and name.
    The identifier of this department instance will be that of Query instance (\link unity::scopes::Query::department_id()\endlink).
    \param query search query (and associated parameters such as filter state) to be executed when this department gets selected

     */
    Department(Query const& query, std::string const& label);

    /**
    \brief Create deparment with given department identifier, search Query and name.
    The query object passed to the ctor will have its target department identifier updated with department_id.
    This constructor is convinient for creating multiple departments that use same query and only need different department identifier.
    \param query search query (and associated parameters such as filter state) to be executed when this department gets selected
     */
    Department(std::string const& department_id, Query const& query, std::string const& label);

    /// @cond
    Department(Department const& other);
    Department(Department&&);
    ~Department();

    Department& operator=(Department const& other);
    Department& operator=(Department&&);
    /// @endcond

    void add_subdepartments(DepartmentList const& departments);

    std::string id() const;
    std::string label() const;
    Query query() const;
    DepartmentList subdepartments() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    std::unique_ptr<internal::DepartmentImpl> p;
};

} // namespace scopes

} // namespace unity

#endif
