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

#include <unity/scopes/internal/CannedQueryImpl.h>
#include <unity/scopes/internal/FilterStateImpl.h>
#include <unity/scopes/internal/JsonCppNode.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/UnityExceptions.h>
#include <unity/scopes/internal/Utils.h>
#include <sstream>
#include <set>

namespace unity
{

namespace scopes
{

namespace internal
{

const std::string CannedQueryImpl::scopes_schema {"scope://"};

CannedQueryImpl::CannedQueryImpl(std::string const& scope_id)
    : scope_id_(scope_id)
{
    if (scope_id_.empty())
    {
        throw InvalidArgumentException("CannedQuery(): scope ID cannot be empty");
    }
}

CannedQueryImpl::CannedQueryImpl(std::string const& scope_id, std::string const& query_str, std::string const& department_id)
    : scope_id_(scope_id),
      query_string_(query_str),
      department_id_(department_id)
{
    if (scope_id_.empty())
    {
        throw InvalidArgumentException("CannedQuery(): scope ID cannot be empty");
    }
}

CannedQueryImpl::CannedQueryImpl(VariantMap const& variant)
{
    auto it = variant.find("scope");
    if (it == variant.end())
    {
        throw InvalidArgumentException("CannedQuery(): scope ID not set");
    }
    scope_id_ = it->second.get_string();
    if (scope_id_.empty())
    {
        throw InvalidArgumentException("CannedQuery(): scope ID cannot be empty");
    }

    it = variant.find("filter_state");
    if (it == variant.end())
    {
        throw InvalidArgumentException("CannedQuery(): filter_state is missing");
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

void CannedQueryImpl::set_department_id(std::string const& dep_id)
{
    department_id_ = dep_id;
}

void CannedQueryImpl::set_query_string(std::string const& query_str)
{
    query_string_ = query_str;
}

void CannedQueryImpl::set_filter_state(FilterState const& filter_state)
{
    filter_state_ = filter_state;
}

std::string CannedQueryImpl::scope_id() const
{
    return scope_id_;
}

std::string CannedQueryImpl::department_id() const
{
    return department_id_;
}

std::string CannedQueryImpl::query_string() const
{
    return query_string_;
}

FilterState CannedQueryImpl::filter_state() const
{
    return filter_state_;
}

VariantMap CannedQueryImpl::serialize() const
{
    VariantMap vm;
    vm["scope"] = scope_id_;
    vm["query_string"] = query_string_;
    vm["department_id"] = department_id_;
    vm["filter_state"] = filter_state_.serialize();
    return vm;
}

std::string CannedQueryImpl::to_uri() const
{
    std::ostringstream s;
    s << scopes_schema << scope_id_;
    s << "?q=" << to_percent_encoding(query_string_);

    if (!department_id_.empty())
    {
        s << "&dep=" << to_percent_encoding(department_id_);
    }

    auto filters_var = filter_state_.serialize();
    if (filters_var.size())
    {
        Variant const var(filters_var);
        internal::JsonCppNode const jstr(var);
        s << "&filters=" << to_percent_encoding(jstr.to_json_string());
    }
    return s.str();
}

CannedQuery CannedQueryImpl::create(VariantMap const& var)
{
    return CannedQuery(new CannedQueryImpl(var));
}

std::string CannedQueryImpl::decode_or_throw(std::string const& value, std::string const& key_name, std::string const& uri)
{
    try
    {
        return from_percent_encoding(value);
    }
    catch (InvalidArgumentException const& e)
    {
        std::stringstream err;
        err << "Failed to decode key '" << key_name << "' of uri '" << uri;
        throw InvalidArgumentException(err.str());
    }
}

CannedQuery CannedQueryImpl::from_uri(std::string const& uri)
{
    size_t pos = scopes_schema.length();
    if (uri.compare(0, pos, scopes_schema) != 0)
    {
        std::stringstream s;
        s << "CannedQuery::from_uri(): unsupported schema '" + uri + "'";
        throw InvalidArgumentException(s.str());
    }

    size_t next = uri.find("?", pos);

    auto scope_id = uri.substr(pos, next - pos);
    if (scope_id.empty())
    {
        std::stringstream s;
        s << "CannedQuery()::from_uri(): scope id is empty in '" << uri << "'";
        throw InvalidArgumentException(s.str());
    }

    CannedQuery q(from_percent_encoding(scope_id));

    if (next != std::string::npos) {
        ++next;
        std::string kv;
        std::istringstream istr(uri.substr(next));
        while (std::getline(istr, kv, '&'))
        {
            auto eqpos = kv.find("=");
            if (eqpos != std::string::npos)
            {
                std::string const key = kv.substr(0, eqpos);
                std::string const val = kv.substr(eqpos + 1);

                if (key == "q")
                {
                    q.set_query_string(decode_or_throw(val, key, uri));
                }
                else if (key == "dep")
                {
                    q.set_department_id(decode_or_throw(val, key, uri));
                }
                else if (key == "filters")
                {
                    auto const fstate_json = decode_or_throw(val, key, uri);
                    internal::JsonCppNode const node(fstate_json);
                    auto const var = node.to_variant();
                    if (var.which() == Variant::Type::Dict)
                    {
                        q.set_filter_state(internal::FilterStateImpl::deserialize(var.get_dict()));
                    }
                    else
                    {
                        std::stringstream s;
                        s << "CannedQuery::from_uri(): invalid filters data for uri: '" << uri << "'";
                        throw InvalidArgumentException(s.str());
                    }
                } // else - unknown keys are ignored
            } // else - the string with no '=' is ignored
        }
    }

    return q;
}

} // namespace internal

} // namespace scopes

} // namespace unity
