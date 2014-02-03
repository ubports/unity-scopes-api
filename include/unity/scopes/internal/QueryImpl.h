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

#ifndef UNITY_SCOPES_INTERNAL_QUERYIMPL_H
#define UNITY_SCOPES_INTERNAL_QUERYIMPL_H

#include <unity/SymbolExport.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/FilterState.h>
#include <string>

namespace unity
{

namespace scopes
{

class Query;
class FilterState;

namespace internal
{

class QueryImpl
{
public:
    explicit QueryImpl(std::string const& scope_name);
    QueryImpl(std::string const& scope_name, std::string const& query_str, std::string const& department_id);
    QueryImpl(VariantMap const& variant);
    QueryImpl(QueryImpl const &other) = default;
    QueryImpl(QueryImpl&&) = default;
    QueryImpl& operator=(QueryImpl const& other) = default;
    QueryImpl& operator=(QueryImpl&&) = default;

    void set_department_id(std::string const& dep_id);
    void set_query_string(std::string const& query_str);
    void set_filter_state(FilterState const& filter_state);
    std::string scope_name() const;
    std::string department_id() const;
    std::string query_string() const;
    FilterState filter_state() const;
    VariantMap serialize() const;
    std::string to_string() const;
    static Query from_string();
    static Query create(VariantMap const& var);

private:
    std::string scope_name_;
    std::string query_string_;
    std::string department_id_;
    FilterState filter_state_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
