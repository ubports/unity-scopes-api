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

#include <unity/scopes/SearchReplyBase.h>

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
\brief SearchReply allows the results of a search query to be sent to the source of a query.
*/

class SearchReply : public virtual SearchReplyBase, public virtual Reply
{
public:
    // @cond
    SearchReply(SearchReply const&) = delete;
    // @endcond

    /**
     \brief Register departments for current search reply and hint the client about current department.
     Current department should in most cases be the one obtained from search request Query::department_id().
     Pass empty string for current_department_id to indicate no active department.
     \param departments a list of departments
     \param current_department_id a department id that should be considered as current
     */
    void register_departments(DepartmentList const& departments, std::string current_department_id = "") override;

    /**
    \brief Create and register a new Category. The category is automatically sent to the source of the query.
    \return Category instance
    */
    Category::SCPtr register_category(std::string const& id,
                                      std::string const& title,
                                      std::string const &icon,
                                      CategoryRenderer const& renderer_template = CategoryRenderer()) override;

    /**
    \brief Register an existing category instance and send it to the source of the query.
    The purpose of this call is to register a category obtained via ReplyBase::push(Category::SCPtr) when aggregating
    results and categories from other scope(s).
    */
    void register_category(Category::SCPtr category) override;

    /**
    \brief Returns an instance of previously registered category.
    \return Category instance or nullptr if category hasn't been registered.
    */
    Category::SCPtr lookup_category(std::string const& id) const override;

    /**
    \brief Sends a single result to the source of a query.
    Any calls to push() after finished() was called are ignored.
    This method throws unity::InvalidArgumentException if pushing a result
    in a Category that wasn't registered before with unity::scopes::SearchReply::register_category().
    \return The return value is true if the result was accepted, false otherwise.
    A false return value is due to either finished() having been called earlier,
    or the client that sent the query having cancelled that query.
    */
    bool push(CategorisedResult const& result) const override;

    /**
    \brief Register Annotation.
    Annotation will get displayed at a next available spot below category registered before.
    Register annotations first before categories to get them displayed
    at the top annotation area. Note: Unity shell can ignore some or all annotations, depending
    on available screen estate.
    */
    bool register_annotation(Annotation const& annotation) const override;

    /**
    \brief Sends all filters and their state to the source of a query.
    \return true if the filters were accepted, false otherwise.
    */
    bool push(Filters const& filters, FilterState const& filter_state) const override;

    /**
    \brief Destroys a Reply.
    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~SearchReply();

protected:
    /// @cond
    SearchReply(internal::ReplyImpl* impl);         // Instantiated only by ReplyImpl
    /// @endcond
    friend class internal::ReplyImpl;
};

} // namespace scopes

} // namespace unity

#endif
