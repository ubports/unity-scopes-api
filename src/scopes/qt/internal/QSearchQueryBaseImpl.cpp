/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#include <unity/scopes/qt/internal/QSearchQueryBaseImpl.h>

#include <unity/scopes/qt/QCannedQuery.h>
#include <unity/scopes/qt/QSearchQueryBaseAPI.h>

#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/SearchQueryBase.h>

#include <unity/UnityExceptions.h>

#include <cassert>
#include <sstream>

using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;
using namespace unity::scopes;

QSearchQueryBaseImpl::QSearchQueryBaseImpl()
    : query_api_(nullptr)
{
}

QCannedQuery QSearchQueryBaseImpl::query() const
{
    assert(query_api_);
    return QCannedQuery(query_api_->query());
}

// TODO change to QSearchMetadata
SearchMetadata QSearchQueryBaseImpl::search_metadata() const
{
    assert(query_api_);
    return query_api_->search_metadata();
}

void QSearchQueryBaseImpl::init(QSearchQueryBaseAPI* query_api)
{
    query_api_ = query_api;
}

QueryCtrlProxy QSearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                               std::string const& query_string,
                                               SearchListenerBase::SPtr const& reply)
{
    return query_api_->subsearch(scope, query_string, reply);
}

QueryCtrlProxy QSearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                               std::string const& query_string,
                                               FilterState const& filter_state,
                                               SearchListenerBase::SPtr const& reply)
{
    return query_api_->subsearch(scope, query_string, filter_state, reply);
}

QueryCtrlProxy QSearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                               std::string const& query_string,
                                               std::string const& department_id,
                                               FilterState const& filter_state,
                                               SearchListenerBase::SPtr const& reply)
{
    return query_api_->subsearch(scope, query_string, department_id, filter_state, reply);
}

QueryCtrlProxy QSearchQueryBaseImpl::subsearch(ScopeProxy const& scope,
                                               std::string const& query_string,
                                               std::string const& department_id,
                                               FilterState const& filter_state,
                                               SearchMetadata const& hints,
                                               SearchListenerBase::SPtr const& reply)
{
    return query_api_->subsearch(scope, query_string, department_id, filter_state, hints, reply);
}
