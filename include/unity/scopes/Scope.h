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

#ifndef UNITY_SCOPES_SCOPE_H
#define UNITY_SCOPES_SCOPE_H

#include <unity/scopes/ObjectProxy.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/SearchListener.h>
#include <unity/scopes/PreviewListener.h>
#include <unity/scopes/ActivationListener.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

class Result;
class FilterState;
class ActionMetadata;
class SearchMetadata;

namespace internal
{
class ScopeImpl;
}

/**
\brief Allows queries to be sent to a scope and results for the query to be retrieved.
*/

class UNITY_API Scope : public virtual ObjectProxy
{
public:
    /**
    \brief Initiates a query.
    The create_query() method expects a SearchListener, which it uses to return
    the results for the query. create_query() may block for some time, for example,
    if the target scope is not running and needs to be started first.
    Results for the query may arrive only after create_query() completes (but may
    also arrive while create_query() is still running).
    */
    QueryCtrlProxy create_query(std::string const& query_string, SearchMetadata const& metadata, SearchListener::SPtr const& reply) const;
    QueryCtrlProxy create_query(std::string const& query_string, FilterState const& filter_state, SearchMetadata const& metadata, SearchListener::SPtr const& reply) const;
    QueryCtrlProxy create_query(std::string const& query_string, std::string const& department_id, FilterState const& filter_state, SearchMetadata const& metadata, SearchListener::SPtr const& reply) const;

    QueryCtrlProxy activate(Result const& result, ActionMetadata const& metadata, ActivationListener::SPtr const& reply) const;

    QueryCtrlProxy perform_action(Result const& result, ActionMetadata const& hints, std::string const& widget_id, std::string const& action_id, ActivationListener::SPtr const& reply) const;

    QueryCtrlProxy preview(Result const& result, ActionMetadata const& metadata, PreviewListener::SPtr const& reply) const;

    /**
    \brief Destroys a Scope.
    Destroying a Scope has no effect on any query that might still be in progress.
    */
    virtual ~Scope();

protected:
    Scope(internal::ScopeImpl* impl);          // Instantiated only by ScopeImpl
    friend class internal::ScopeImpl;

private:
    internal::ScopeImpl* fwd() const;
};

} // namespace scopes

} // namespace unity

#endif
