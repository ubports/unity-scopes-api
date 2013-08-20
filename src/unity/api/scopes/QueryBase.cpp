/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/QueryBase.h>

#include <unity/api/scopes/internal/QueryBaseImpl.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

QueryBase::QueryBase(string const&) :   // TODO: why pass a string here that isn't used? Scope name?
    p(new internal::QueryBaseImpl)
{
}

QueryBase::~QueryBase() noexcept
{
}

QueryCtrlProxy QueryBase::create_subquery(string const& query_string,
                                          ScopeProxy const& scope,
                                          shared_ptr<ReplyBase> const& reply)
{
    return p->create_subquery(query_string, scope, reply);
}

void QueryBase::cancel()
{
    p->cancel();
}

} // namespace scopes

} // namespace api

} // namespace unity
