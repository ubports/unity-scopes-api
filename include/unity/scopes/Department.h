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

#include <unity/scopes/Variant.h>
#include <unity/util/DefinesPtrs.h>
#include <list>
#include <string>
#include <memory>

namespace unity
{

namespace scopes
{

class CannedQuery;
class Department;

/*! \typedef DepartmentList
\brief List of departments (see unity::scopes::Department)
*/
typedef std::list<Department> DepartmentList;

namespace internal
{
class DepartmentImpl;
}

/**
\brief A department with optional sub-departments.
*/
class Department final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Department);
    /// @endcond

    /**
    \brief Create a department with the given canned query and name.

    The identifier of this department instance is that of the `query` instance (\link CannedQuery::department_id()\endlink).
    \param query The query (and associated parameters, such as filter state) to be executed when the user
    selects this department.
    \param label The display name of this department.
    */
    Department(CannedQuery const& query, std::string const& label);

    /**
    \brief Create a department with the given department identifier, canned query, and name.

    The canned query's target department identifier is updated with department_id.
    This constructor is convenient for creating multiple departments that use the same query and only
    need a different department identifier.
    \param department_id The department identifier.
    \param query The canned query (and associated parameters, such as filter state) to be executed when the user
    selects this department.
    \param label The display name of this department.
    */
    Department(std::string const& department_id, CannedQuery const& query, std::string const& label);

    /**
    \brief Create a department with the given department identifier, canned query, name, and sub-departments.

    The canned query's target department identifier is updated with department_id.
    This constructor is convenient for creating multiple departments that use the same query and only
    need a different department identifier.
    \param department_id The department identifier.
    \param query The canned query (and associated parameters, such as filter state) to be executed when the user
    selects this department.
    \param label The display name of this department.
    \param subdepartments The sub-departments of this department.
     */
    Department(std::string const& department_id, CannedQuery const& query, std::string const& label, DepartmentList const& subdepartments);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    Department(Department const& other);
    Department(Department&&);
    Department& operator=(Department const& other);
    Department& operator=(Department&&);
    //@}

    /// @cond
    ~Department();
    /// @endcond

    /**
    \brief Set sub-departments of this department.
    \param departments The list of sub-departments.
    */
    void set_subdepartments(DepartmentList const& departments);

    /**
     \brief Set the alternate label (plural of the normal label) of this department.

     The alternate label should express the plural "all" form of the normal label. For example,
     if the normal label is "Books", then the alternate label should be "All Books". The alternate label
     needs to be provided for the current department only.
     \param label The alternate label to display when showing plural form of this department's name.
    */
    void set_alternate_label(std::string const& label);

    /**
     \brief Sets has_subdepartments flag of this department to true.

     This flag is a display hint for the Shell that indicates if this department has sub-departments and as such should be displayed
     in a way that suggests further navigation to the user.
     Setting this flag is not needed when sub-departments have been added with set_subdepartments() method.
     */
    void set_has_subdepartments();

    /**
    \brief Get the identifier of this department.
    \return The department identifier.
    */
    std::string id() const;

    /**
    \brief Get the label of this department.
    \return The department label.
    */
    std::string label() const;

    /**
    \brief Get the alternate label of this department.

    Return the alternate label of this department. The alternate label expresses the plural "all" form of the normal label.
    For example, if the normal label is "Books", then the alternate label is "All Books". Note that alternate label
    and can be empty - in that case the normal label should be displayed instead.

    \return The alternate label.
     */
    std::string alternate_label() const;

    /**
    \brief Get the canned query associated with this department.
    \return The canned query for this department.
    */
    CannedQuery query() const;

    /**
    \brief Get list of sub-departments of this department.
    \return The list of sub-departments.
    */
    DepartmentList subdepartments() const;

    /**
     \brief Check if this department has subdepartments or has_subdepartments flag is set
     \return true if this deparment has subdepartments or has_subdepartments flag is set
     */
    bool has_subdepartments() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

private:
    std::unique_ptr<internal::DepartmentImpl> p;
};

} // namespace scopes

} // namespace unity

#endif
