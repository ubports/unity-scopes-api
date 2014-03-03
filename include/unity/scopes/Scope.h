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
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/PreviewListenerBase.h>
#include <unity/scopes/ActivationListenerBase.h>
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

class Scope : public virtual ObjectProxy
{
public:
    /**
    \brief Initiates a search query.

    The search() method expects a SearchListenerBase, which it uses to return
    the results for the query. search() may block for some time, for example,
    if the target scope is not running and needs to be started first.
    Results for the query may begin to arrive only after search() completes (but may
    also arrive while search() is still running).

    \param query_string search string
    \param metadata additional data to pass to scope
    \param reply search response handler
    \return query handler
    */
    QueryCtrlProxy search(std::string const& query_string, SearchMetadata const& metadata, SearchListenerBase::SPtr const& reply) const;

    /**
    \brief Initiates a search query (overloaded method).

    This method has same synopsis as previous method, but it takes additional unity::scopes::FilterState argument.

    \param query_string search string
    \param filter_state state of filters
    \param metadata additional data to pass to scope
    \param reply search response handler
    \return query handler
     */
    QueryCtrlProxy search(std::string const& query_string, FilterState const& filter_state, SearchMetadata const& metadata, SearchListenerBase::SPtr const& reply) const;

    /**
    \brief Initiates a search query (overloaded method).

    This method has same synopsis as previous method, but it takes additional department identifier argument.

    \param query_string search string
    \param department_id identifier of a department to search
    \param filter_state state of filters
    \param metadata additional data to pass to scope
    \param reply search response handler
    \return query handler
     */
    QueryCtrlProxy search(std::string const& query_string, std::string const& department_id, FilterState const& filter_state, SearchMetadata const& metadata, SearchListenerBase::SPtr const& reply) const;

    /**
     \brief Initiates activation of a search result.
     \param result activated result
     \param metadata additional data to pass to scope
     \param reply activation response handler
     \return query handler
     */
    QueryCtrlProxy activate(Result const& result, ActionMetadata const& metadata, ActivationListenerBase::SPtr const& reply) const;

    /**
     \brief Initiates activation of a preview action.
     \param result Result that was previewed.
     \param metadata additional data to pass to scope
     \param widget_id identifier of 'actions' widget of activated action
     \param action_id identifier of an action to activate
     \param reply activation response handler
     \return query handler
     */
    QueryCtrlProxy perform_action(Result const& result, ActionMetadata const& metadata, std::string const& widget_id, std::string const& action_id,
            ActivationListenerBase::SPtr const& reply) const;

    /**
     \brief Initiates preview request.
     \param result Result to be previewed
     \param metadata additional data to pass to scope
     \param reply preview response handler
     */
    QueryCtrlProxy preview(Result const& result, ActionMetadata const& metadata, PreviewListenerBase::SPtr const& reply) const;

    /**
    \brief Destroys a Scope.

    Destroying a Scope has no effect on any query that might still be in progress.
    */
    virtual ~Scope();

protected:
    /// @cond
    Scope(internal::ScopeImpl* impl);          // Instantiated only by ScopeImpl
    friend class internal::ScopeImpl;
    /// @endcond

private:
    internal::ScopeImpl* fwd() const;
};

} // namespace scopes

} // namespace unity

#endif
