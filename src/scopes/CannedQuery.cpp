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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */


#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/internal/CannedQueryImpl.h>

namespace unity
{

namespace scopes
{

CannedQuery::CannedQuery(std::string const& scope_id)
    : p(new internal::CannedQueryImpl(scope_id))
{
}

CannedQuery::CannedQuery(std::string const& scope_id, std::string const& query_str, std::string const& department_id)
    : p(new internal::CannedQueryImpl(scope_id, query_str, department_id))
{
}

/// @cond
CannedQuery::CannedQuery(CannedQuery const &other)
    : p(new internal::CannedQueryImpl(*(other.p)))
{
}

CannedQuery::CannedQuery(internal::CannedQueryImpl *impl)
    : p(impl)
{
}

CannedQuery::CannedQuery(CannedQuery&&) = default;

CannedQuery::~CannedQuery() = default;

CannedQuery& CannedQuery::operator=(CannedQuery const& other)
{
    if (this != &other)
    {
        p.reset(new internal::CannedQueryImpl(*(other.p)));
    }
    return *this;
}

CannedQuery& CannedQuery::operator=(CannedQuery&&) = default;

VariantMap CannedQuery::serialize() const
{
    return p->serialize();
}
/// @endcond

void CannedQuery::set_department_id(std::string const& dep_id)
{
    p->set_department_id(dep_id);
}

void CannedQuery::set_query_string(std::string const& query_str)
{
    p->set_query_string(query_str);
}

void CannedQuery::set_filter_state(FilterState const& filter_state)
{
    p->set_filter_state(filter_state);
}

std::string CannedQuery::scope_id() const
{
    return p->scope_id();
}

std::string CannedQuery::department_id() const
{
    return p->department_id();
}

std::string CannedQuery::query_string() const
{
    return p->query_string();
}

FilterState CannedQuery::filter_state() const
{
    return p->filter_state();
}

std::string CannedQuery::to_string() const
{
    return p->to_string();
}

CannedQuery CannedQuery::from_string()
{
    return internal::CannedQueryImpl::from_string();
}

} // namespace scopes

} // namespace unity
