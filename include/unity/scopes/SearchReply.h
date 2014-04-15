/*
 * Copyright (C) 2013 Canonical Ltd
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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_SCOPES_SEARCHREPLY_H
#define UNITY_SCOPES_SEARCHREPLY_H

#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/Reply.h>

namespace unity
{

namespace scopes
{

class CategorisedResult;
class Annotation;

/**
\brief Allows the results of a search query to be sent to the query source.
*/

class SearchReply : public virtual Reply
{
public:
    /**
     \brief Register departments for the current search reply and provide the current department.

     Departments are optional. If scope supports departments, it is expected to register a DepartmentList in one of the following ways,
     depending on what is the current department user is facing:
     <ul>
     <li>if user is facing the root of the departments hierarchy (i.e. no department is selected), just register a single department that corresponds to "all
     departments", and add to it sub-departments corresponding to top-level departments. For every department on the list call unity::scopes::Department::set_has_subdepartments() if it
     has subdepartments.
     <li>if user is visiting a department, you need to create parent department - current department - sub-departments hierarchy as follows:
        <ul>
        <li>create a Department instance for current department and append all relevant sub-departments of it using unity::scopes::Department::set_subdepartments() method.
        For every sub-department use unity::scopes::Department::set_has_subdepartments() if it has subdepartments.
        <li>create a Department instance that is a parent of current department and add current department to it using set_subdepartments().
        <li>register a unity::scopes::DepartmentList that has just the parent department in it.
        </ul>
     </ul>

     For example, assuming the user is visiting a "History" department in "Books", and "History" has sub-departments such as "World War Two" and "Ancient", the code
     that registers departments for current search in "History" may look like this:
     \code{.cpp}
     unity::scopes::Department books("books", query, "Books"); // the parent of "History"
     unity::scopes::DepartmentList history_depts({"history", query, "History"}, {"ww2", query, "World War Two"}, {"ancient", query, "Ancient"});
     books.set_subdepartments(history_depts);
     unity::scopes::DepartmentList books_depts({books});
     reply->register_departments(books_depts, "history");
     \endcode

     current_department_id should in most cases be the department returned by unity::scopes::CannedQuery::department_id().
     Pass an empty string for current_department_id to indicate no active department.
     \param departments A list of departments.
     \param current_department_id A department id that should be considered current.
     */
    virtual void register_departments(DepartmentList const& departments, std::string current_department_id = "") = 0;

    /**
    \brief Register an existing category instance and send it to the source of the query.

    The purpose of this call is to register a category obtained via ReplyBase::push(Category::SCPtr) when aggregating
    results and categories from other scope(s).
    */
    virtual Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const &icon,
                                              CategoryRenderer const& renderer_template = CategoryRenderer()) = 0;

    /**
    \brief Returns a previously registered category.
    \return The category instance or `nullptr` if the category does not exist registered.
    */
    virtual void register_category(Category::SCPtr category) = 0;

    /**
    \brief Returns a previously registered category.
    \return The category instance or `nullptr` if the category does not exist.
    */
    virtual Category::SCPtr lookup_category(std::string const& id) = 0;

    /**
    \brief Sends a single result to the source of a query.

    Any calls to push() after finished() was called are ignored.
    \return The return value is true if the result was accepted, false otherwise.
    A false return value can be due to finished() having been called earlier,
    or the client that sent the query having cancelled that query. The return
    value is false also if the query has a cardinality limit and is reached
    or exceeded. (The return value is false for the last valid push and
    subsequent pushes.)
    */
    virtual bool push(CategorisedResult const& result) = 0;

    /**
    \brief Register an annotation.

    The annotation will be rendered at a next available spot below any category registered earlier.
    To render annotations in the top annotation area, call register_annotation() before
    registering any categories.
    \note The Unity shell can ignore some or all annotations, depending on available screen real estate.
    */
    virtual bool register_annotation(Annotation const& annotation) = 0;

    /**
    \brief Sends all filters and their state to the source of a query.
    \return True if the filters were accepted, false otherwise.
    */
    virtual bool push(Filters const& filters, FilterState const& filter_state) = 0;

    /**
    \brief Destroys a Reply.

    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~SearchReply();

protected:
    /// @cond
    SearchReply();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif
