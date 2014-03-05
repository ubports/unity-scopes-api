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

#ifndef UNITY_SCOPES_SEARCHLISTENERBASE_H
#define UNITY_SCOPES_SEARCHLISTENERBASE_H

#include <unity/scopes/ListenerBase.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/Annotation.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/FilterBase.h>

#include <string>

namespace unity
{

namespace scopes
{

class CategorisedResult;
class FilterState;

/**
\brief Abstract base interface that a scope specializes to
receive the results of a query.

An instance of this interface must be passed to Scope::create_query().
Results for the query are delivered to the instance by the scopes run
time by invoking the appropriate push method.

If a scope throw an exception from one of the push() methods, the scopes
run time calls ListenerBase::finished() with an 'Error' reason.
*/

class SearchListenerBase : public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(SearchListenerBase);
    UNITY_DEFINES_PTRS(SearchListenerBase);

    virtual ~SearchListenerBase();
    /// @endcond

    /**
    \brief Called at most once by the scopes run time for a list of departments returned by a query.

    The default implementation does nothing.
    */
    virtual void push(DepartmentList const& departments, std::string const& current_department_id);

    /**
    \brief Called once by the scopes run time for each result that is returned by a query().
    */
    virtual void push(CategorisedResult result) = 0;

    /**
    \brief Called once by the scopes run time for each annotation that is returned by a query().

    The default implementation does nothing.
    */
    virtual void push(Annotation annotation);

    /**
    \brief Called once by the scopes run time for each category that is returned by a query().

    The default implementation does nothing. Receipt of categories may be interleaved with
    the receipt of results, that is, there is no guarantee that the complete set of categories
    will be provided before the first query result.
    */
    virtual void push(Category::SCPtr category);

    /**
     \brief Called once by the scopes to send all the filters and their state.

     The default implementation does nothing.
     */
    virtual void push(Filters const& filters, FilterState const& filter_state);

protected:
    /// @cond
    SearchListenerBase();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif
