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

#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/SearchMetadata.h>

#include <unity/scopes/internal/QueryBaseImpl.h>
#include <unity/scopes/internal/SearchQueryBaseImpl.h>

using namespace std;

namespace unity
{

namespace scopes
{

/// @cond
SearchQueryBase::~SearchQueryBase()
{
}

SearchQueryBase::SearchQueryBase(CannedQuery const& query, SearchMetadata const& metadata)
    : QueryBase(new internal::SearchQueryBaseImpl(query, metadata))
{
}

CannedQuery SearchQueryBase::query() const
{
    return fwd()->query();
}

SearchMetadata SearchQueryBase::search_metadata() const
{
    return fwd()->search_metadata();
}

/// @endcond

/// @cond
// These methods are documented in the header. The @cond is here to suppress
// a bogus warning from doxygen about undocumented methods. (Doxygen gets
// confused by the typedef for ScopeProxy.)

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                          string const& query_string,
                                          SearchListenerBase::SPtr const& reply)
{
    return fwd()->subsearch(scope, query_string, "", FilterState(), nullptr, fwd()->search_metadata(), reply);
}

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                          std::string const& query_string,
                                          FilterState const& filter_state,
                                          SearchListenerBase::SPtr const& reply)
{
    return fwd()->subsearch(scope, query_string, "", filter_state, nullptr, fwd()->search_metadata(), reply);
}

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                          std::string const& query_string,
                                          std::string const& department_id,
                                          FilterState const& filter_state,
                                          SearchListenerBase::SPtr const& reply)
{
    return fwd()->subsearch(scope, query_string, department_id, filter_state, nullptr, fwd()->search_metadata(), reply);
}

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                          std::string const& query_string,
                                          std::string const& department_id,
                                          FilterState const& filter_state,
                                          SearchMetadata const& hints,
                                          SearchListenerBase::SPtr const& reply)
{
    return fwd()->subsearch(scope, query_string, department_id, filter_state, nullptr, hints, reply);
}

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                   std::string const& query_string,
                                   std::string const& department_id,
                                   FilterState const& filter_state,
                                   Variant const& user_data,
                                   SearchMetadata const& hints,
                                   SearchListenerBase::SPtr const& reply)
{
    return fwd()->subsearch(scope, query_string, department_id, filter_state, std::unique_ptr<Variant>(new Variant(user_data)), hints, reply);
}

void SearchQueryBase::set_department_id(std::string const& department_id)
{
    fwd()->set_department_id(department_id);
}

std::string SearchQueryBase::department_id() const
{
    return fwd()->department_id();
}

internal::SearchQueryBaseImpl* SearchQueryBase::fwd() const
{
    return dynamic_cast<internal::SearchQueryBaseImpl*>(p.get());
}

/// @endcond

} // namespace scopes

} // namespace unity
