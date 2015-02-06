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

#pragma once

#include <unity/util/DefinesPtrs.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariantMap>
#pragma GCC diagnostic pop
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtCore/QScopedPointer>

#include <memory>

namespace unity
{

namespace scopes
{

namespace qt
{

class QDepartment;
class QCannedQuery;

namespace internal
{
class QDepartmentImpl;
class QSearchReplyImpl;
}

/*! \typedef QDepartmentList
\brief List of departments (see unity::scopes::qt::QDepartment)
*/
typedef QList<QSharedPointer<QDepartment const>> QDepartmentList;

/**
\brief A department with optional sub-departments.
*/
class QDepartment final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QDepartment);
    /// @endcond

    /**
    \brief Create a department with the given canned query and name.

    The identifier of this department instance is that of the `query` instance (\link
    QCannedQuery::department_id()\endlink).
    \param query The query (and associated parameters, such as filter state) to be executed when the user
    selects this department.
    \param label The display name of this department.
    */
    static QDepartment::UPtr create(QCannedQuery const& query, QString const& label);

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
    static QDepartment::UPtr create(QString const& department_id, QCannedQuery const& query, QString const& label);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QDepartment(QDepartment const& other);
    QDepartment(QDepartment&&);
    QDepartment& operator=(QDepartment const& other);
    QDepartment& operator=(QDepartment&&);
    //@}

    /// @cond
    ~QDepartment();
    /// @endcond

    /**
    \brief Set sub-departments of this department.
    \param departments The list of sub-departments.
    */
    void set_subdepartments(QDepartmentList const& departments);

    /**
    \brief Add sub-department to this department.
    \param department The subdepartment instance.
    */
    void add_subdepartment(QDepartment::SCPtr const& department);

    /**
     \brief Set the alternate label (plural of the normal label) of this department.

     The alternate label should express the plural "all" form of the normal label. For example,
     if the normal label is "Books", then the alternate label should be "All Books". The alternate label
     needs to be provided for the current department only.
     \param label The alternate label to display when showing plural form of this department's name.
    */
    void set_alternate_label(QString const& label);

    /**
     \brief Sets has_subdepartments flag of this department.

     This flag is a display hint for the Shell that indicates if this department has sub-departments and as such should
     be displayed
     in a way that suggests further navigation to the user.
     Setting this flag is not needed when sub-departments have been added with set_subdepartments() method.
     Setting this flag to false after adding sub-departments with set_subdepartments() throws unity::LogicException.

     \throws unity::LogicException if called with false after adding sub-departments with
     unity::scopes::Department::set_subdepartments()
     */
    void set_has_subdepartments(bool subdepartments = true);

    /**
    \brief Get the identifier of this department.
    \return The department identifier.
    */
    QString id() const;

    /**
    \brief Get the label of this department.
    \return The department label.
    */
    QString label() const;

    /**
    \brief Get the alternate label of this department.

    Return the alternate label of this department. The alternate label expresses the plural "all" form of the normal
    label.
    For example, if the normal label is "Books", then the alternate label is "All Books". Note that alternate label
    and can be empty - in that case the normal label should be displayed instead.

    \return The alternate label.
     */
    QString alternate_label() const;

    /**
    \brief Get the canned query associated with this department.
    \return The canned query for this department.
    */
    QCannedQuery query() const;

    /**
    \brief Get list of sub-departments of this department.
    \return The list of sub-departments.
    */
    QDepartmentList subdepartments() const;

    /**
     \brief Check if this department has subdepartments or has_subdepartments flag is set
     \return true if this deparment has subdepartments or has_subdepartments flag is set
     */
    bool has_subdepartments() const;

    /// @cond
    QVariantMap serialize() const;
    /// @endcond

private:
    /// @cond
    QDepartment(internal::QDepartmentImpl* impl);
    std::unique_ptr<internal::QDepartmentImpl> p;
    friend class internal::QDepartmentImpl;
    friend class internal::QSearchReplyImpl;
    /// @endcond
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity
