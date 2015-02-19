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

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

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
    /// @cond
    NONCOPYABLE(QSearchQueryBaseImpl);
    UNITY_DEFINES_PTRS(QSearchQueryBaseImpl);
    /// @endcond

    QSearchQueryBaseImpl();
    ~QSearchQueryBaseImpl() = default;

    /**
     \brief Get a canned query for this search request.

     \return The canned query.
     \throws unity::LogicException if the canned query was not initialized (was default-constructed).
     */
    QCannedQuery query() const;

    /**
     \brief Get metadata for this search request.
     \return The search metadata.
     \throws unity::LogicException if search metadata was not initialized (was default-constructed).
    */
    unity::scopes::SearchMetadata search_metadata() const;

    /** @name Subquery methods
    The subsearch() methods are for use by aggregating scopes.
    When an aggregator passes a query to its child scopes, it should
    use subsearch() instead of the normal Scope::search()
    that would be called by a client. subsearch() takes care
    of automatically forwarding query cancellation to child scopes.
    This means that there is no need for an aggregating scope to
    explicitly forward cancellation to child scopes
    when its QueryBase::cancelled() method is called by the scopes
    run time.
    */
    //{@
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
