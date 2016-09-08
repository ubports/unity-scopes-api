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

#pragma once

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

namespace experimental
{
class Annotation;
}

/**
\brief Allows the results of a search query to be sent to the query source.
*/

class SearchReply : public virtual Reply
{
public:
    /**
     \brief Register departments for the current search reply and provide the current department.

     Departments are optional. If scope supports departments, it is expected to register departments on every search as follows:

      <ul>
      <li>create a Department node for current department and attach to it a list of its subdepartments (unless current department is a leaf department) using
      unity::scopes::Department::set_subdepartments() method. For every subdepartment on the list set "has_subdepartments" flag if applicable.
      <li>provide an alternate label for current department with unity::scopes::Department::set_alternate_label().
      <li>create a Department node for parent of current department (if applicable - not when in root department), and attach current Department node to it with
      unity::scopes::Department::set_subdepartments() method.
      <li>register the parent department with unity::scopes::SearchReply::register_departments().
      </ul>

     For example, assuming the user is visiting a "History" department in "Books", and "History" has sub-departments such as "World War Two" and "Ancient", the code
     that registers departments for current search in "History" may look like this:
     \code{.cpp}
     unity::scopes::Department::SPtr books = move(unity::scopes::Department::create("books", query, "Books")); // the parent of "History"
     unity::scopes::Department::SPtr history = move(unity::scopes::Department::create("history", query, "History"));
     unity::scopes::DepartmentList history_depts({
                                                 move(unity::scopes::Department::create("ww2", query, "World War Two")),
                                                 move(unity::scopes::Department::create("ancient", query, "Ancient"))});
     history->set_subdepartments(history_depts);
     books->set_subdepartments({history});
     reply->register_departments(books);
     \endcode

     Current department should be the department returned by unity::scopes::CannedQuery::department_id(). Empty department id denotes
     the root deparment.

     \param parent The parent department of current department, or current one if visiting root department.
     \throws unity::LogicException if departments are invalid (nullptr passed, current department not present in the parent's tree, duplicated department ids present in the tree).
     */
    virtual void register_departments(Department::SCPtr const& parent) = 0;

    /**
    \brief Register new category and send it to the source of the query.

    \param id The identifier of the category
    \param title The title of the category
    \param icon The icon of the category
    \param renderer_template The renderer template to be used for results in this category

    \return The category instance
    \throws unity::scopes::InvalidArgumentException if category with that id has already been registered.

    \note The UI uses category IDs for incremental rendering when refreshing search results. For this to work
    correctly, you need to make sure that you use the same category ID for different queries if the search
    results apply to the same category as for a previous search.
    */
    virtual Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const& icon,
                                              CategoryRenderer const& renderer_template = CategoryRenderer()) = 0;

    /**
    \brief Register an existing category instance and send it to the source of the query.

    The purpose of this call is to register a category obtained via SearchListenerBase::push(Category::SCPtr const&) when aggregating
    results and categories from other scope(s).

    \throws unity::InvalidArgumentException if category is already registered.
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
    \brief Push an annotation.

    The annotation will be rendered at a next available spot below the category of last pushed result.
    To render an annotation in the top annotation area, push it before pushing search results.
    \note The Unity shell can ignore some or all annotations, depending on available screen real estate.
    */
    virtual bool push(experimental::Annotation const& annotation) = 0;

    /**
    \brief Sends all filters and their states to the source of a query.
    \deprecated Sending filter state back to the UI is deprecated and will be removed from future releases. Please use the push(Filters const&) method instead.
    \return True if the filters were accepted, false otherwise.
    */
    virtual bool push(Filters const& filters, FilterState const& filter_state) = 0;

    /**
    \brief Destroys a Reply.

    If a Reply goes out of scope without a prior call to finished(), the destructor implicitly calls finished().
    */
    virtual ~SearchReply();

    /**
    \brief Register new category and send it to the source of the query.

    \param id The identifier of the category
    \param title The title of the category
    \param icon The icon of the category
    \param query Query to perform when expanding this category
    \param renderer_template The renderer template to be used for results in this category

    \return The category instance
    \throws unity::scopes::InvalidArgumentException if category with that id has already been registered.
    */
    virtual Category::SCPtr register_category(std::string const& id,
                                              std::string const& title,
                                              std::string const& icon,
                                              CannedQuery const& query,
                                              CategoryRenderer const& renderer_template = CategoryRenderer()) = 0;

    /**
    \brief Push the results that were produced by the most recent surfacing query.

    The runtime automatically saves the results of the most recent surfacing query.
    If a scope cannot produce a result for a surfacing query (presumably, due to
    connectivity problems), calling push_surfacing_results_from_cache() pushes
    the results that were produced by the most recent successful surfacing query
    from the cache. If a scope cannot produce surfacing results, it can call this
    method to "replay" the previous results. In turn, this avoids the user being
    presented with an empty screen if he/she swipes to the scope while the device
    does not have connectivity.

    This method has an effect only if called for a surfacing query (that is, a
    query with an empty query string). If called for a non-empty query, it does
    nothing.

    You must call this method before calling finished(), otherwise no cached results
    will be pushed (push_surfacing_results_from_cache() implicitly calls finished()).
    */
    virtual void push_surfacing_results_from_cache() = 0;

    /**
    \brief Sends all filter definitions to the source of a query.
    \return True if the filters were accepted, false otherwise.
    */
    virtual bool push(Filters const& filters) = 0;

protected:
    /// @cond
    SearchReply();
    /// @endcond
};

} // namespace scopes

} // namespace unity
