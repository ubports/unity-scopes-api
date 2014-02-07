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

#ifndef UNITY_SCOPES_SEARCHQUERYBASE_H
#define UNITY_SCOPES_SEARCHQUERYBASE_H

#include <unity/scopes/QueryBase.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/SearchListener.h>

#include <unity/SymbolExport.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class QueryBaseImpl;
class QueryObject;

} // namespace internal

// Abstract server-side base interface for a query that is executed inside a scope.

// TODO: documentation

class UNITY_API SearchQuery: public QueryBase
{
public:
    NONCOPYABLE(SearchQuery);
    UNITY_DEFINES_PTRS(SearchQuery);

    /// Invoked when a SearchQuery is run, use the provided proxy to push results
    virtual void run(SearchReplyProxy const& reply) = 0;         // Called by the run time to start this query

    // Create a sub-query.
    QueryCtrlProxy create_subquery(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   VariantMap const& hints,
                                   SearchListener::SPtr const& reply);
    QueryCtrlProxy create_subquery(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   FilterState const& filter_state,
                                   VariantMap const& hints,
                                   SearchListener::SPtr const& reply);
    QueryCtrlProxy create_subquery(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   std::string const& department_id,
                                   FilterState const& filter_state,
                                   VariantMap const& hints,
                                   SearchListener::SPtr const& reply);

    /// @cond
    virtual ~SearchQuery();
    /// @endcond

protected:
    /// @cond
    SearchQuery();
    /// @endcond
};

} // namespace scopes

} // namespace unity

#endif
