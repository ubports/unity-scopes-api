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

#ifndef UNITY_API_SCOPES_SCOPEPROXY_H
#define UNITY_API_SCOPES_SCOPEPROXY_H

#include <unity/api/scopes/ObjectProxy.h>
#include <unity/api/scopes/ReplyBase.h>

#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class ScopeProxyImpl;
}

/**
\brief Allows queries to be sent to a scope and results for the query to be retrieved.
*/

class UNITY_API ScopeProxy : public ObjectProxy
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(ScopeProxy);
    /// @endcond

    /**
    \brief Initiates a query.
    The query() method expects an instance derived from ReplyBase, which it uses to return
    the results for the query. query() is an asynchronous method, that is, it returns immediately;
    results for the query typically will arrive only after the query() method completes (but may
    also arrive while the query() method is still running.
    */
    void query(std::string const& q, ReplyBase::SPtr const& reply);

    /**
    \brief Destroys a ScopeProxy.
    Destroying a ScopeProxy has no effect on any query that might still be in progress.
    */
    virtual ~ScopeProxy() noexcept;

private:
    ScopeProxy(internal::ScopeProxyImpl* impl);          // Only instantiable by ScopeProxyImpl
    friend class internal::ScopeProxyImpl;

    std::shared_ptr<internal::ScopeProxyImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
