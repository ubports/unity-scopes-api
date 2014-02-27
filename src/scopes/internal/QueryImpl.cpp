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

#include <unity/scopes/internal/QueryImpl.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/Query.h>
#include <unity/UnityExceptions.h>
#include <sstream>

namespace unity
{

namespace scopes
{

namespace internal
{

QueryImpl::QueryImpl(std::string const& scope_id)
    : scope_id_(scope_id)
{
    if (scope_id_.empty())
    {
        throw InvalidArgumentException("Query(): scope name cannot be empty");
    }
}

QueryImpl::QueryImpl(std::string const& scope_id, std::string const& query_str, std::string const& department_id)
    : scope_id_(scope_id),
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
    scope_id_ = it->second.get_string();
    if (scope_id_.empty())
    {
        throw InvalidArgumentException("Query(): scope name cannot be empty");
    }

    it = variant.find("filter_state");
    if (it == variant.end())
    {
        throw InvalidArgumentException("Query(): filter_state is missing");
    }
    filter_state_ = internal::FilterStateImpl::deserialize(it->second.get_dict());

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

std::string QueryImpl::scope_id() const
{
    return scope_id_;
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
    vm["scope"] = scope_id_;
    vm["query_string"] = query_string_;
    vm["department_id"] = department_id_;
    vm["filter_state"] = filter_state_.serialize();
    return vm;
}

std::string QueryImpl::to_string() const
{
    std::ostringstream s;
    s << "scope://" << scope_id_;
    s << "?q=" << query_string_; // FIXME: escape
    if (!department_id_.empty())
    {
        s << "&department=" << department_id_;
    }
    return s.str();
}

Query QueryImpl::create(VariantMap const& var)
{
    return Query(new QueryImpl(var));
}

Query QueryImpl::from_string()
{
    //TODO
    Query q("");
    return q;
}

} // namespace internal

} // namespace scopes

} // namespace unity
