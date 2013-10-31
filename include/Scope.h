/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_SCOPE_H
#define UNITY_API_SCOPES_SCOPE_H

#include <unity/api/scopes/ObjectProxy.h>
#include <unity/api/scopes/QueryCtrlProxyFwd.h>
#include <unity/api/scopes/ReplyBase.h>
#include <unity/api/scopes/ScopeProxyFwd.h>
#include <unity/api/scopes/VariantMap.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class ScopeImpl;
}

/**
\brief Allows queries to be sent to a scope and results for the query to be retrieved.
*/

class UNITY_API Scope : public ObjectProxy
{
public:
    /**
    \brief Initiates a query.
    The query() method expects an instance derived from ReplyBase, which it uses to return
    the results for the query. query() is an asynchronous method, that is, it returns immediately;
    results for the query typically will arrive only after the query() method completes (but may
    also arrive while the query() method is still running.
    */
    QueryCtrlProxy create_query(std::string const& q, VariantMap const& hints, ReplyBase::SPtr const& reply) const;

    /**
    \brief Destroys a Scope.
    Destroying a Scope has no effect on any query that might still be in progress.
    */
    virtual ~Scope() noexcept;

private:
    Scope(internal::ScopeImpl* impl);          // Instantiable only by ScopeImpl
    friend class internal::ScopeImpl;

    std::shared_ptr<internal::ScopeImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
