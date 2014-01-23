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

#ifndef UNITY_SCOPES_SEARCHLISTENER_H
#define UNITY_SCOPES_SEARCHLISTENER_H

#include <unity/scopes/ListenerBase.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/Annotation.h>
#include <unity/scopes/FilterBase.h>

#include <string>

namespace unity
{

namespace scopes
{

class CategorisedResult;
class FilterState;

class UNITY_API SearchListener : public ListenerBase
{
public:
    /// @cond
    NONCOPYABLE(SearchListener);
    UNITY_DEFINES_PTRS(SearchListener);

    virtual ~SearchListener() noexcept;
    /// @endcond

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

    If push() throws an exception, the scopes run time calls finished() with an 'Error' reason.
    */
    virtual void push(Category::SCPtr category);

    /**
     \brief Called once by the scopes to send all the filters and their state.
     The default implementation does nothing.
     */
    virtual void push(Filters const& filters, FilterState const& filter_state);

protected:
    /// @cond
    SearchListener();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif
