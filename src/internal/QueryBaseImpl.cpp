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

#include <scopes/internal/QueryBaseImpl.h>

#include <scopes/QueryCtrl.h>
#include <scopes/Scope.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

QueryBaseImpl::QueryBaseImpl()
{
}

QueryBaseImpl::~QueryBaseImpl() noexcept
{
}

QueryCtrlProxy QueryBaseImpl::create_subquery(ScopeProxy const& scope,
                                              string const& query_string,
                                              VariantMap const& hints,
                                              shared_ptr<SearchListener> const& reply)
{
    // Forward the create request to the child scope and remember the control.
    // This allows cancel() to forward incoming cancellations to subqueries
    // without intervention from the scope application code.
    QueryCtrlProxy qcp = scope->create_query(query_string, hints, reply);
    subqueries_.push_back(qcp);
    return qcp;
}

void QueryBaseImpl::cancel()
{
    for (auto ctrl : subqueries_)
    {
        ctrl->cancel(); // Forward the cancellation to any subqueries that might be active
    }
    // We release the memory for the subquery controls here. That's just a micro-optimization
    // because this QueryBase will be destroyed shortly anyway, once the cancelled() method
    // of the application returns. (Not deallocating here would work too.)
    vector<QueryCtrlProxy>().swap(subqueries_);
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
