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

#ifndef UNITY_API_SCOPES_QUERYBASE_H
#define UNITY_API_SCOPES_QUERYBASE_H

#include <scopes/QueryCtrlProxyFwd.h>
#include <scopes/ReplyProxyFwd.h>
#include <scopes/ScopeProxyFwd.h>
#include <scopes/Variant.h>

#include <unity/SymbolExport.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace api
{

namespace scopes
{

class SearchListener;

namespace internal
{

class QueryBaseImpl;
class QueryObject;

} // namespace internal

// Abstract server-side base interface for a query that is executed inside a scope.

// TODO: documentation

class UNITY_API QueryBase
{
public:
    NONCOPYABLE(QueryBase);
    UNITY_DEFINES_PTRS(QueryBase);

    virtual void cancelled() = 0;                          // Originator cancelled the query
    virtual void run(ReplyProxy const& reply) = 0;         // Called by the run time to start this query

    // Create a sub-query.
    QueryCtrlProxy create_subquery(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   VariantMap const& hints,
                                   std::shared_ptr<SearchListener> const& reply);

    /// @cond
    virtual ~QueryBase() noexcept;
    /// @endcond

protected:
    /// @cond
    QueryBase();
    /// @endcond

private:
    void cancel();
    friend class internal::QueryObject;       // So QueryObject can call cancel()

    std::unique_ptr<internal::QueryBaseImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
