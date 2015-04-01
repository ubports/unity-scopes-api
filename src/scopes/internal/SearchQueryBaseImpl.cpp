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
      canned_query_(query),
      search_metadata_(metadata),
      valid_(true)
{
}

CannedQuery SearchQueryBaseImpl::query() const
{
    return canned_query_;
}

SearchMetadata SearchQueryBaseImpl::search_metadata() const
{
    return search_metadata_;
}

QueryCtrlProxy SearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                              std::set<std::string> const& keywords,
                                              std::string const& query_string,
                                              std::string const& department_id,
                                              FilterState const& filter_state,
                                              SearchMetadata const& metadata,
                                              SearchListenerBase::SPtr const& reply)
{
    if (!scope)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): scope cannot be nullptr");
    }
    if (!reply)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): reply cannot be nullptr");
    }

    auto query_ctrl = check_for_query_loop(scope, reply);
    if (query_ctrl)
    {
        return query_ctrl;  // Loop was detected, return dummy QueryCtrlProxy.
    }

    // Insert the keywords used to aggregated this child scope into its search metadata
    query_ctrl = insert_aggregated_keywords(scope, reply, const_cast<SearchMetadata&>(metadata));
    if (query_ctrl)
    {
        return query_ctrl;  // Child scope is disabled, return dummy QueryCtrlProxy.
    }

    // scope_impl can be nullptr if we use a mock scope: TypedScopeFixture<testing::Scope>
    // If so, we call the normal search without passing the history through because
    // we don't need loop detection for mock scopes.
    auto scope_impl = dynamic_pointer_cast<ScopeImpl>(scope);
    if (scope_impl)
    {
        query_ctrl = scope_impl->search(query_string, department_id, filter_state, metadata, history_, reply);
    }
    else
    {
        query_ctrl = scope->search(query_string, department_id, filter_state, metadata, reply);
    }

    lock_guard<mutex> lock(mutex_);
    subqueries_.push_back(query_ctrl);  // Remember subsearch in case we get a cancel() later that we need to forward.
    return query_ctrl;
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

void SearchQueryBaseImpl::set_child_scopes_func(std::function<ChildScopeList()> const& child_scopes_func)
{
    lock_guard<mutex> lock(mutex_);
    child_scopes_func_ = child_scopes_func;
}

bool SearchQueryBaseImpl::valid() const
{
    lock_guard<mutex> lock(mutex_);
    return valid_;
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

    lock_guard<mutex> lock(mutex_);

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
        ctrl_proxy = make_shared<QueryCtrlImpl>(nullptr, nullptr);  // Dummy proxy in already-cancelled state
        BOOST_LOG_SEV(logger, Logger::Warning)
            << "query loop for query \"" << canned_query_.query_string()
            << "\", client: " << get<0>(tuple)
            << ", aggregator: " << get<1>(tuple)
            << ", receiver: " << get<2>(tuple) << endl;
    }
    else
    {
        history_.push_back(tuple);
    }

    return ctrl_proxy;  // null proxy if there was no loop
}

QueryCtrlProxy SearchQueryBaseImpl::insert_aggregated_keywords(ScopeProxy const& scope,
                                                               SearchListenerBase::SPtr const& reply,
                                                               SearchMetadata& metadata)
{
    lock_guard<mutex> lock(mutex_);

    // If we don't have child_scopes_func_, this means the scope was not invocated via the middleware
    // (e.g. during unit testing). Just return and continue with the subsearch here.
    if (!child_scopes_func_)
    {
        return nullptr;
    }

    // Initialize (lazy) our child_scopes_ map
    if (child_scopes_.empty())
    {
        // We use the child scopes list to find and insert aggregated keywords into subsearch metadatas.
        // Therefore, here we put the child scopes into a map of id:ChildScope for quick reference.
        auto child_scopes = child_scopes_func_();
        for (auto const& child : child_scopes)
        {
            child_scopes_.insert(std::make_pair(child.id, child));
        }
    }

    // Check that the target child scope is in our child_scopes_ map
    if (child_scopes_.find(scope->identity()) == child_scopes_.end())
    {
        // The child scope was not specified in the child scopes list, we should not continue with the subsearch
        reply->finished(CompletionDetails(CompletionDetails::OK,
                                          "empty result set as target child scope \"" + scope->identity()
                                          + "\" was not specified in this aggregator's child scopes list"));
        return make_shared<QueryCtrlImpl>(nullptr, nullptr);  // Dummy proxy in already-cancelled state
    }

    auto child_scope = child_scopes_.at(scope->identity());

    // Check that the target child scope is enabled
    if (!child_scope.enabled)
    {
        // The child scope is disabled, we should not continue with the subsearch
        reply->finished(CompletionDetails(CompletionDetails::OK,
                                          "empty result set as target child scope \"" + scope->identity()
                                          + "\" is currently disabled"));
        return make_shared<QueryCtrlImpl>(nullptr, nullptr);  // Dummy proxy in already-cancelled state
    }

    // Set aggregated keywords
    metadata.set_aggregated_keywords(child_scope.keywords);
    return nullptr;
}

} // namespace internal

} // namespace scopes

} // namespace unity
