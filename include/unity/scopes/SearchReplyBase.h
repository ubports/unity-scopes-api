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

#ifndef UNITY_SCOPES_SEARCHREPLY_BASE_H
#define UNITY_SCOPES_SEARCHREPLY_BASE_H

#include <unity/scopes/ReplyBase.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>

namespace unity
{

namespace scopes
{

class CategorisedResult;
class Annotation;

/**
\brief Abstract base to allow the results of a search query to be sent to the source of a query.
*/

class SearchReplyBase : public virtual ReplyBase
{
public:
    /// @cond
    SearchReplyBase(SearchReplyBase const&) = delete;
    /// @endcond

    /**
     \brief Register departments for current search reply and hint the client about current department.
     Current department should in most cases be the one obtained from search request Query::department_id().
     Pass empty string for current_department_id to indicate no active department.
     \param departments a list of departments
     \param current_department_id a department id that should be considered as current
     */
    virtual void register_departments(DepartmentList const& departments, std::string current_department_id = "") = 0;

    /**
    \brief Create and register a new Category. The category is automatically sent to the source of the query.
    \return Category instance
    */
    virtual Category::SCPtr register_category(std::string const& id,
                                      std::string const& title,
                                      std::string const &icon,
                                      CategoryRenderer const& renderer_template = CategoryRenderer()) = 0;

    /**
    \brief Register an existing category instance and send it to the source of the query.
    The purpose of this call is to register a category obtained via ReplyBase::push(Category::SCPtr) when aggregating
    results and categories from other scope(s).
    */
    virtual void register_category(Category::SCPtr category) = 0;

    /**
    \brief Returns an instance of previously registered category.
    \return Category instance or nullptr if category hasn't been registered.
    */
    virtual Category::SCPtr lookup_category(std::string const& id) const = 0;

    // TODO: document return value from push()
    /**
    \brief Sends a single result to the source of a query.
    Any calls to push() after finished() was called are ignored.
    \return The return value is true if the result was accepted, false otherwise.
    A false return value is due to either finished() having been called earlier,
    or the client that sent the query having cancelled that query.
    */
    virtual bool push(CategorisedResult const& result) const = 0;

    /**
    \brief Register Annotation.
    Annotation will get displayed at a next available spot below category registered before.
    Register annotations first before categories to get them displayed
    at the top annotation area. Note: Unity shell can ignore some or all annotations, depending
    on available screen estate.
    */
    virtual bool register_annotation(Annotation const& annotation) const = 0;

    /**
    \brief Sends all filters and their state to the source of a query.
    \return true if the filters were accepted, false otherwise.
    */
    virtual bool push(Filters const& filters, FilterState const& filter_state) const = 0;

    /**
    \brief Destroys a Reply.
    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~SearchReplyBase();

protected:
    /// @cond
    SearchReplyBase();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif
