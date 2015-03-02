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

#pragma once

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/internal/QueryBaseImpl.h>
#include <unity/scopes/QueryCtrlProxyFwd.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/SearchListenerBase.h>
#include <unity/scopes/SearchMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class SearchQueryBaseImpl : public QueryBaseImpl
{
public:
    SearchQueryBaseImpl(CannedQuery const& query, SearchMetadata const& metadata);
    CannedQuery query() const;
    SearchMetadata search_metadata() const;

    void set_department_id(std::string const& department_id);
    std::string department_id() const;

    void set_client_id(const std::string& id);

    typedef std::tuple<std::string, std::string, std::string> HistoryData;
    typedef std::vector<HistoryData> History;

    void set_history(History const& h);

    QueryCtrlProxy subsearch(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   std::string const& department_id,
                                   FilterState const& filter_state,
                                   std::unique_ptr<Variant> user_data,
                                   SearchMetadata const& metadata,
                                   SearchListenerBase::SPtr const& reply);

    void cancel() override;
    bool valid() const override;

private:
    CannedQuery const canned_query_;
    SearchMetadata const search_metadata_;

    mutable std::mutex mutex_;
    bool valid_;
    std::string department_id_;
    std::string client_id_;
    History history_;
    std::vector<QueryCtrlProxy> subqueries_;

    QueryCtrlProxy check_for_query_loop(ScopeProxy const& scope, SearchListenerBase::SPtr const& reply);
};

} // namespace internal

} // namespace scopes

} // namespace unity
