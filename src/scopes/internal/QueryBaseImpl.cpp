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

#include <unity/scopes/internal/QueryBaseImpl.h>

#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/UnityExceptions.h>

#include <iostream>
#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

QueryBaseImpl::QueryBaseImpl()
    : valid_(true)
{
}

QueryBaseImpl::~QueryBaseImpl()
{
}

QueryCtrlProxy QueryBaseImpl::subsearch(ScopeProxy const& scope,
                                              string const& query_string,
                                              SearchListenerBase::SPtr const& reply)
{
    assert(search_metadata_);

    if (!scope)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): scope cannot be nullptr");
    }
    if (!reply)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): reply cannot be nullptr");
    }

    // Forward the create request to the child scope and remember the control.
    // This allows cancel() to forward incoming cancellations to subqueries
    // without intervention from the scope application code.
    QueryCtrlProxy qcp = scope->search(query_string, *search_metadata_, reply);

    lock_guard<mutex> lock(mutex_);
    subqueries_.push_back(qcp);
    return qcp;
}

QueryCtrlProxy QueryBaseImpl::subsearch(ScopeProxy const& scope,
                                              std::string const& query_string,
                                              FilterState const& filter_state,
                                              SearchListenerBase::SPtr const& reply)
{
    assert(search_metadata_);

    if (!scope)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): scope cannot be nullptr");
    }
    if (!reply)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): reply cannot be nullptr");
    }

    QueryCtrlProxy qcp = scope->search(query_string, filter_state, *search_metadata_, reply);

    lock_guard<mutex> lock(mutex_);
    subqueries_.push_back(qcp);
    return qcp;
}

QueryCtrlProxy QueryBaseImpl::subsearch(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   std::string const& department_id,
                                   FilterState const& filter_state,
                                   SearchListenerBase::SPtr const& reply)
{
    assert(search_metadata_);

    if (!scope)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): scope cannot be nullptr");
    }
    if (!reply)
    {
        throw InvalidArgumentException("QueryBase::subsearch(): reply cannot be nullptr");
    }

    QueryCtrlProxy qcp = scope->search(query_string, department_id, filter_state, *search_metadata_, reply);

    lock_guard<mutex> lock(mutex_);
    subqueries_.push_back(qcp);
    return qcp;
}

QueryCtrlProxy QueryBaseImpl::subsearch(ScopeProxy const& scope,
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

    QueryCtrlProxy qcp = scope->search(query_string, department_id, filter_state, metadata, reply);

    lock_guard<mutex> lock(mutex_);
    subqueries_.push_back(qcp);
    return qcp;
}

void QueryBaseImpl::set_metadata(SearchMetadata const& metadata)
{
    search_metadata_.reset(new SearchMetadata(metadata));
}

void QueryBaseImpl::cancel()
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

void QueryBaseImpl::set_department_id(std::string const& department_id)
{
    department_id_ = department_id;
}

std::string QueryBaseImpl::department_id() const
{
    return department_id_;
}

bool QueryBaseImpl::valid() const
{
    lock_guard<mutex> lock(mutex_);
    return valid_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
