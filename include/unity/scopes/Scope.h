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

#include <unity/scopes/ActivationListenerBase.h>
#include <unity/scopes/ChildScope.h>
#include <unity/scopes/Object.h>
#include <unity/scopes/PreviewListenerBase.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

class Result;
class FilterState;
class ActionMetadata;
class SearchMetadata;

/**
\brief Allows queries, preview requests, and activation requests to be sent to a scope.
*/

class Scope : public virtual Object
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
    \param reply The callback object to receive replies
    \return A proxy that permits cancellation of this request
    */
    virtual QueryCtrlProxy search(std::string const& query_string,
                                  SearchMetadata const& metadata,
                                  SearchListenerBase::SPtr const& reply) = 0;

    /**
    \brief Initiates a search query (overloaded method).

    This method has same synopsis as previous method, but it takes additional unity::scopes::FilterState argument.

    \param query_string search string
    \param filter_state state of filters
    \param metadata additional data to pass to scope
    \param reply The callback object to receive replies
    \return A proxy that permits cancellation of this request
    */
    virtual QueryCtrlProxy search(std::string const& query_string,
                                  FilterState const& filter_state,
                                  SearchMetadata const& metadata,
                                  SearchListenerBase::SPtr const& reply) = 0;

    /**
    \brief Initiates a search query (overloaded method).

    This method has same synopsis as previous method, but it takes additional department identifier argument.

    \param query_string search string
    \param department_id identifier of a department to search
    \param filter_state state of filters
    \param metadata additional data to pass to scope
    \param reply The callback object to receive replies
    \return query handler
    */
    virtual QueryCtrlProxy search(std::string const& query_string,
                                  std::string const& department_id,
                                  FilterState const& filter_state,
                                  SearchMetadata const& metadata,
                                  SearchListenerBase::SPtr const& reply) = 0;

    /**
    \brief Initiates activation of a search result.
    \param result activated result
    \param metadata additional data to pass to scope
    \param reply The callback object to receive replies
    \return A proxy that permits cancellation of this request
    */
    virtual QueryCtrlProxy activate(Result const& result,
                                    ActionMetadata const& metadata,
                                    ActivationListenerBase::SPtr const& reply) = 0;

    /**
    \brief Initiates activation of a preview action.
    \param result Result that was previewed.
    \param metadata additional data to pass to scope
    \param widget_id identifier of 'actions' widget of activated action
    \param action_id identifier of an action to activate
    \param reply The callback object to receive replies
    \return A proxy that permits cancellation of this request
    */
    virtual QueryCtrlProxy perform_action(Result const& result,
                                          ActionMetadata const& metadata,
                                          std::string const& widget_id,
                                          std::string const& action_id,
                                          ActivationListenerBase::SPtr const& reply) = 0;

    /**
    \brief Initiates preview request.
    \param result Result to be previewed
    \param metadata additional data to pass to scope
    \param reply The callback object to receive replies
    \return A proxy that permits cancellation of this request
    */
    virtual QueryCtrlProxy preview(Result const& result,
                                   ActionMetadata const& metadata,
                                   PreviewListenerBase::SPtr const& reply) = 0;

    /**
    \brief Destroys a Scope.

    Destroying a Scope has no effect on any query that might still be in progress.
    */
    virtual ~Scope();

    /**
    \brief Returns a list of child scopes aggregated by this scope in user-defined order.
    \return The list of child scopes aggregated by this scope in user-defined order.
    */
    virtual ChildScopeList child_scopes_ordered() = 0;

    /**
    \brief Sets the ordered list of child scopes aggregated by this scope.
    \note The only time this call will return false is if the scope cannot write to its config directory.
    This should not happen in real-world usage, but if it does, check the log for more detail.
    \param child_scopes_ordered The ordered list of child scopes aggregated by this scope.
    \return True if the list was successfully set.
    */
    virtual bool set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered) = 0;

    /**
    \brief Initiates a search query (overloaded method).

    This method has same synopsis as previous search method, but it takes additional user_data argument.

    \param query_string search string
    \param department_id identifier of a department to search
    \param filter_state state of filters
    \param user_data arbitrary data
    \param metadata additional data to pass to scope
    \param reply The callback object to receive replies
    \return query handler
    */
    virtual QueryCtrlProxy search(std::string const& query_string,
                                  std::string const& department_id,
                                  FilterState const& filter_state,
                                  Variant const& user_data,
                                  SearchMetadata const& metadata,
                                  SearchListenerBase::SPtr const& reply) = 0;

protected:
    /// @cond
    Scope();
    /// @endcond
};

} // namespace scopes

} // namespace unity
