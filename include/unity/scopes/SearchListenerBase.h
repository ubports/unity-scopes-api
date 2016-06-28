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

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/Department.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <string>

namespace unity
{

namespace scopes
{

namespace experimental
{
class Annotation;
}

/**
\brief Abstract base interface for a client to receive the results of a query.

An instance of this interface must be passed to Scope::search().
Results for the query are delivered to the client by the scopes run
time by invoking the appropriate push method.

If the implementation of a push method throws an exception, the scopes
runtime calls ListenerBase::finished() with an 'Error' status.

\see ListenerBase
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
    \brief Called at most once by the scopes runtime for a tree of departments returned by a query.

    The default implementation does nothing.
    */
    virtual void push(Department::SCPtr const& parent);

    /**
    \brief Called once by the scopes runtime for each result that is returned by a query().
    */
    virtual void push(CategorisedResult result) = 0;

    /**
    \brief Called once by the scopes runtime for each annotation that is returned by a query().

    The default implementation does nothing.
    */
    virtual void push(experimental::Annotation annotation);

    /**
    \brief Called once by the scopes runtime for each category that is returned by a query().

    Receipt of categories may be interleaved with
    the receipt of results, that is, there is no guarantee that the complete set of categories
    will be provided before the first query result.

    The default implementation does nothing.
    */
    virtual void push(Category::SCPtr const& category);

    /**
    \brief Called once by the scopes to send all the filters and their state.

    \deprecated Please override the push(Filters& const) method instead. This method will be removed from future releases.

    The default implementation does nothing.
    */
    virtual void push(Filters const& filters, FilterState const& filter_state);

    /**
    \brief Called once by the scope to send all filters and their states.

    The default implementation does nothing.
    */
    virtual void push(Filters const& filters);

protected:
    /// @cond
    SearchListenerBase();
    /// @endcond
};

} // namespace scopes

} // namespace unity
