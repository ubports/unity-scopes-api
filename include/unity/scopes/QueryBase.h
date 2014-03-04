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

#ifndef UNITY_SCOPES_QUERYBASE_H
#define UNITY_SCOPES_QUERYBASE_H

#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

class QueryMetadata;

namespace internal
{

class QueryBaseImpl;
class QueryObject;
class ScopeObject;

namespace smartscopes
{

class SSQueryObject;

} // namespace smartscopes

} // namespace internal


/**
\brief Abstract server-side base interface for a query that is executed inside a scope.
\see SearchQueryBase, PreviewQueryBase, ActivationQueryBase
*/

class QueryBase
{
public:
    /// @cond
    NONCOPYABLE(QueryBase);
    UNITY_DEFINES_PTRS(QueryBase);
    /// @endcond

    /**
    \brief Called by the scopes run time when the query originator
    cancels a query.

    Your implementation of this method should ensure that the scope stops
    processing the current query as soon as possible. Any calls to ReplyBase::push()
    once a query is cancelled are ignored, so continuing to push after cancellation
    only wastes CPU cycles.
    */
    virtual void cancelled() = 0;                          // Originator cancelled the query

    /// @cond
    virtual ~QueryBase();
    /// @endcond

protected:
    /// @cond
    QueryBase();
    void cancel();
    std::unique_ptr<internal::QueryBaseImpl> p;
    /// @endcond

private:
    void set_metadata(QueryMetadata const& metadata);

    friend class internal::QueryObject;                    // So QueryObject can call cancel()
    friend class internal::smartscopes::SSQueryObject;     // So SSQueryObject can call cancel()
    friend class internal::ScopeObject;                    // So ScopeObject can call set_metadata()
};

} // namespace scopes

} // namespace unity

#endif
