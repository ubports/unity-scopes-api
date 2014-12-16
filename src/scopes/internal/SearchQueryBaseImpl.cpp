/*
 * Copyright (C) 2014 Canonical Ltd
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
 *              Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/internal/SearchQueryBaseImpl.h>

#include <unity/scopes/internal/QueryCtrlImpl.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/SearchMetadataImpl.h>
#include <unity/scopes/ScopeExceptions.h>

#include <unity/UnityExceptions.h>

#include <algorithm>
#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

using namespace std;

SearchQueryBaseImpl::SearchQueryBaseImpl(CannedQuery const& query, SearchMetadata const& metadata)
    : QueryBaseImpl(),
      valid_(true),
      canned_query_(query),
      search_metadata_(metadata)
{
}

CannedQuery SearchQueryBaseImpl::query() const
{
    return canned_query_;
}

SearchMetadata SearchQueryBaseImpl::search_metadata() const
{
    lock_guard<mutex> lock(mutex_);
    return search_metadata_;
}

QueryCtrlProxy SearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                              string const& query_string,
                                              SearchListenerBase::SPtr const& reply)
{
    function<QueryCtrlProxy()> search;
    auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
    if (scope_impl)
    {
        search = [&]{ return scope_impl->search(query_string, "", FilterState(), search_metadata_, history_, reply); };
    }
    else
    {
        // scope_impl can be nullptr if we use a mock scope: TypedScopeFixture<testing::Scope>
        // If so, we call the normal search without passing the history through, because
        // we don't need loop detection for mock scopes.
        search = [&]{ return scope->search(query_string, "", FilterState(), search_metadata_, reply); };
    }
    return do_subsearch(scope, reply, search);
}

QueryCtrlProxy SearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                              std::string const& query_string,
                                              FilterState const& filter_state,
                                              SearchListenerBase::SPtr const& reply)
{
    function<QueryCtrlProxy()> search;
    auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
    if (scope_impl)
    {
        search = [&]{ return scope_impl->search(query_string, "", filter_state, search_metadata_, history_, reply); };
    }
    else
    {
        search = [&]{ return scope->search(query_string, "", filter_state, search_metadata_, reply); };
    }
    return do_subsearch(scope, reply, search);
}

QueryCtrlProxy SearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   std::string const& department_id,
                                   FilterState const& filter_state,
                                   SearchListenerBase::SPtr const& reply)
{
    function<QueryCtrlProxy()> search;
    auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
    if (scope_impl)
    {
        search = [&]{ return scope_impl->search(query_string, department_id, filter_state, search_metadata_, history_, reply); };
    }
    else
    {
        search = [&]{ return scope->search(query_string, department_id, filter_state, search_metadata_, reply); };
    }
    return do_subsearch(scope, reply, search);
}

QueryCtrlProxy SearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                              std::string const& query_string,
                                              std::string const& department_id,
                                              FilterState const& filter_state,
                                              SearchMetadata const& metadata,
                                              SearchListenerBase::SPtr const& reply)
{
    function<QueryCtrlProxy()> search;
    auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
    if (scope_impl)
    {
        search = [&]{ return scope_impl->search(query_string, department_id, filter_state, metadata, history_, reply); };
    }
    else
    {
        search = [&]{ return scope->search(query_string, department_id, filter_state, metadata, reply); };
    }
    return do_subsearch(scope, reply, search);
}

void SearchQueryBaseImpl::cancel()
{
    lock_guard<mutex> lock(mutex_);

    if (!valid_)
    {
        return;
    }
    valid_ = false;
    for (auto& ctrl : subqueries_)
    {
        ctrl->cancel(); // Forward the cancellation to any subqueries that might be active
    }
    // We release the memory for the subquery controls here. That's just a micro-optimization
    // because this QueryBase will be destroyed shortly anyway, once the cancelled() (and possibly
    // run()) methods of the application return. (Not deallocating here would work too.)
    vector<QueryCtrlProxy>().swap(subqueries_);
}

void SearchQueryBaseImpl::set_department_id(std::string const& department_id)
{
    lock_guard<mutex> lock(mutex_);
    department_id_ = department_id;
}

std::string SearchQueryBaseImpl::department_id() const
{
    lock_guard<mutex> lock(mutex_);
    return department_id_;
}

void SearchQueryBaseImpl::set_client_id(std::string const& id)
{
    lock_guard<mutex> lock(mutex_);
    client_id_ = id;
}

void SearchQueryBaseImpl::set_history(History const& h)
{
    lock_guard<mutex> lock(mutex_);
    history_ = h;
}

bool SearchQueryBaseImpl::valid() const
{
    lock_guard<mutex> lock(mutex_);
    return valid_;
}

void SearchQueryBaseImpl::check_subsearch_params(ScopeProxy const& scope, SearchListenerBase::SPtr const& reply)
{
    if (!scope)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): scope cannot be nullptr");
    }
    if (!reply)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): reply cannot be nullptr");
    }
}

// Check if this query has been through this aggregator before by checking the
// history. The history is a list of <client, aggregator, receiver> scope ID tuples.
// If a query loops around to this aggregator, and has been sent to the same child
// previously on behalf of the same client, we immediately call finished to indicate
// an empty result set. If a loop is detected, we return a dummy QueryCtrl. Otherwise,
// we add the current <client, aggregator, receiver> tuple to the history before forwarding the search.
//
// Note that we do this unconditionally, whether the receiver is a leaf or an aggregator.
// This means that an aggregator can ask a *leaf scope* more than once for data.
// This works because normal searches do not check the query history (only
// subsearches do). But an aggregator cannot ask another *aggregator* more
// than once for data, even if that would not create a loop.
//
// It is possible for an aggregator to send a query to itself, but only once.

QueryCtrlProxy SearchQueryBaseImpl::check_for_query_loop(ScopeProxy const& scope,
                                                         SearchListenerBase::SPtr const& reply)
{
    shared_ptr<QueryCtrlImpl> ctrl_proxy;

    assert(!mutex_.try_lock());

    HistoryData tuple = make_tuple(client_id_, canned_query_.scope_id(), scope->identity());
    auto it = find(history_.begin(), history_.end(), tuple);
    if (it != history_.end())
    {
        // Query has been here before from the same client and with the same child scope as the target.
        reply->finished(CompletionDetails(CompletionDetails::OK,
                                          "empty result set due to aggregator loop or repeated query on aggregating scope "
                                          + get<1>(tuple)));
        auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
        auto logger = scope_impl->runtime()->logger();
        ctrl_proxy = make_shared<QueryCtrlImpl>(nullptr, nullptr, logger);  // Dummy proxy in already-cancelled state
        BOOST_LOG_SEV(logger, Logger::Warning)
            << "query loop for query \"" << canned_query_.query_string()
            << "\", client: " << get<0>(tuple) << ", aggregator: " << get<1>(tuple) << ", receiver: " << get<2>(tuple) << endl;
    }
    else
    {
        history_.push_back(tuple);
    }

    return ctrl_proxy;  // null proxy if there was no loop
}

// Add the query ctrl for a subsearch to the list of subqueries so, if a cancel()
// arrives, we can forward the cancel to all the subsearches.

void SearchQueryBaseImpl::remember_subsearch(QueryCtrlProxy const& qcp)
{
    assert(!mutex_.try_lock());

    subqueries_.push_back(qcp);
}

QueryCtrlProxy SearchQueryBaseImpl::do_subsearch(ScopeProxy const& scope,
                                                 SearchListenerBase::SPtr const& reply,
                                                 function<QueryCtrlProxy()> search_func)
{
    check_subsearch_params(scope, reply);

    lock_guard<mutex> lock(mutex_);

    auto query_ctrl = check_for_query_loop(scope, reply);
    if (query_ctrl)
    {
        return query_ctrl;            // Loop was detected, return dummy QueryCtrlProxy.
    }
    query_ctrl = search_func();       // Run the real search on the child scope.
    remember_subsearch(query_ctrl);
    return query_ctrl;
}

} // namespace internal

} // namespace scopes

} // namespace unity
