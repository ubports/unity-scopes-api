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

#pragma once

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>

#include <unity/scopes/qt/QCannedQuery.h>
#include <unity/scopes/qt/QSearchReplyProxy.h>

namespace unity
{

namespace scopes
{

class SearchMetadata;
class CannedQuery;

namespace qt
{

class QSearchQueryBase;
class QSearchQueryBaseAPI;

namespace internal
{

class QSearchQueryBaseImpl
{
    friend QSearchQueryBase;

public:
    NONCOPYABLE(QSearchQueryBaseImpl);
    UNITY_DEFINES_PTRS(QSearchQueryBaseImpl);

    QSearchQueryBaseImpl();
    ~QSearchQueryBaseImpl() = default;

    QCannedQuery query() const;
    unity::scopes::SearchMetadata search_metadata() const;

    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             SearchListenerBase::SPtr const& reply);
    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             FilterState const& filter_state,
                             SearchListenerBase::SPtr const& reply);
    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             std::string const& department_id,
                             FilterState const& filter_state,
                             SearchListenerBase::SPtr const& reply);
    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                             std::string const& query_string,
                             std::string const& department_id,
                             FilterState const& filter_state,
                             SearchMetadata const& hints,
                             SearchListenerBase::SPtr const& reply);

protected:
    QSearchQueryBaseAPI* query_api_;

private:
    void init(QSearchQueryBaseAPI* query_api);
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity
