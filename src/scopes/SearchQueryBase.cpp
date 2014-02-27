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

using namespace std;

namespace unity
{

namespace scopes
{

/// @cond
SearchQueryBase::SearchQueryBase() : QueryBase()
{
}

SearchQueryBase::~SearchQueryBase()
{
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
    return p->subsearch(scope, query_string, reply);
}

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                          std::string const& query_string,
                                          FilterState const& filter_state,
                                          SearchListenerBase::SPtr const& reply)
{
    return p->subsearch(scope, query_string, filter_state, reply);
}

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                          std::string const& query_string,
                                          std::string const& department_id,
                                          FilterState const& filter_state,
                                          SearchListenerBase::SPtr const& reply)
{
    return p->subsearch(scope, query_string, department_id, filter_state, reply);
}

QueryCtrlProxy SearchQueryBase::subsearch(ScopeProxy const& scope,
                                          std::string const& query_string,
                                          std::string const& department_id,
                                          FilterState const& filter_state,
                                          SearchMetadata const& hints,
                                          SearchListenerBase::SPtr const& reply)
{
    return p->subsearch(scope, query_string, department_id, filter_state, hints, reply);
}
/// @endcond

} // namespace scopes

} // namespace unity
