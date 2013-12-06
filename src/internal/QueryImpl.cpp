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

#include <scopes/internal/QueryImpl.h>
#include <scopes/Query.h>
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

QueryImpl::QueryImpl(std::string const& scope_name)
    : scope_name_(scope_name)
{
    if (scope_name_.empty())
    {
        throw InvalidArgumentException("Query(): scope name cannot be empty");
    }
}

QueryImpl::QueryImpl(std::string const& scope_name, std::string const& query_str, std::string const& department_id)
    : scope_name_(scope_name),
      query_string_(query_str),
      department_id_(department_id)
{
}

QueryImpl::QueryImpl(VariantMap const& variant)
{
    auto it = variant.find("scope");
    if (it == variant.end())
    {
        throw InvalidArgumentException("Query(): scope name not set");
    }
    scope_name_ = it->second.get_string();
    if (scope_name_.empty())
    {
        throw InvalidArgumentException("Query(): scope name cannot be empty");
    }

    it = variant.find("department_id");
    if (it != variant.end())
    {
        department_id_ = it->second.get_string();
    }

    it = variant.find("query_string");
    if (it != variant.end())
    {
        query_string_ = it->second.get_string();
    }
}

bool QueryImpl::operator==(QueryImpl const& other) const
{
    if (scope_name_ == other.scope_name_ &&
        query_string_ == other.query_string_ &&
        department_id_ == other.department_id_)
    {
        return true;
    }
    return false;
}

void QueryImpl::set_department_id(std::string const& dep_id)
{
    department_id_ = dep_id;
}

void QueryImpl::set_query_string(std::string const& query_str)
{
    query_string_ = query_str;
}

void QueryImpl::set_filter_state(FilterState const& filter_state)
{
    filter_state_ = filter_state;
}

std::string QueryImpl::scope_name() const
{
    return scope_name_;
}

std::string QueryImpl::department_id() const
{
    return department_id_;
}

std::string QueryImpl::query_string() const
{
    return query_string_;
}

FilterState QueryImpl::filter_state() const
{
    return filter_state_;
}

VariantMap QueryImpl::serialize() const
{
    VariantMap vm;
    vm["scope"] = scope_name_;
    vm["query_string"] = query_string_;
    vm["department_id"] = department_id_;
    //TODO filters
    return vm;
}

std::string QueryImpl::to_string() const
{
    std::ostringstream s;
    s << "scope://" << scope_name_;
    s << "?q=" << query_string_; // FIXME: escape
    if (!department_id_.empty())
    {
        s << "&department=" << department_id_;
    }
    return s.str();
}

Query QueryImpl::from_string()
{
    //TODO
    Query q("");
    return q;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
