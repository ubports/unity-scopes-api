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


#include <scopes/Query.h>
#include <scopes/internal/QueryImpl.h>

namespace unity
{

namespace api
{

namespace scopes
{

Query::Query(std::string const& scope_name)
    : p(new internal::QueryImpl(scope_name))
{
}

Query::Query(std::string const& scope_name, std::string const& query_str, std::string const& department_id)
    : p(new internal::QueryImpl(scope_name, query_str, department_id))
{
}

Query::Query(VariantMap const& variant)
    : p(new internal::QueryImpl(variant))
{
}

Query::Query(Query const &other)
    : p(new internal::QueryImpl(*(other.p)))
{
}

Query::Query(Query&&) = default;

Query& Query::operator=(Query const& other)
{
    if (this != &other)
    {
        p.reset(new internal::QueryImpl(*(other.p)));
    }
    return *this;
}

Query& Query::operator=(Query&&) = default;

bool Query::operator==(Query const& other) const
{
    return *p == *(other.p);
}

bool Query::operator!=(Query const& other) const
{
    return !(this->operator==(other));
}

void Query::set_department_id(std::string const& dep_id)
{
    p->set_department_id(dep_id);
}

void Query::set_query_string(std::string const& query_str)
{
    p->set_query_string(query_str);
}

void Query::set_filter_state(FilterState const& filter_state)
{
    //TODO
}

std::string Query::scope_name() const
{
    return p->scope_name();
}

std::string Query::department_id() const
{
    return p->department_id();
}

std::string Query::query_string() const
{
    return p->query_string();
}

FilterState Query::filter_state() const
{
    //TODO
    return FilterState();
}

VariantMap Query::serialize() const
{
    return p->serialize();
}

std::string Query::to_string() const
{
    return p->to_string();
}

Query Query::from_string()
{
    return internal::QueryImpl::from_string();
}

} // namespace scopes

} // namespace api

} // namespace unity
