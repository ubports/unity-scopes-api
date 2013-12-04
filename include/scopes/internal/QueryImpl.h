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

#ifndef UNITY_INTERNAL_QUERYIMPL_H
#define UNITY_INTERNAL_QUERYIMPL_H

#include <unity/SymbolExport.h>
#include <scopes/Variant.h>
#include <string>

namespace unity
{

namespace api
{

namespace scopes
{

class Query;

namespace internal
{

class QueryImpl
{
public:
    QueryImpl(std::string const& scope_name);
    QueryImpl(std::string const& scope_name, std::string const& query_str, std::string const& department_id);
    QueryImpl(QueryImpl const &other) = default;
    QueryImpl(QueryImpl&&) = default;
    QueryImpl& operator=(QueryImpl const& other) = default;
    QueryImpl& operator=(QueryImpl&&) = default;
    bool operator==(QueryImpl const& other) const;

    void set_department_id(std::string const& dep_id);
    void set_query_string(std::string const& query_str);
    std::string scope_name() const;
    std::string department_id() const;
    std::string query_string() const;
    VariantMap serialize() const;
    std::string to_string() const;
    static Query from_string();

private:
    std::string scope_name_;
    std::string query_string_;
    std::string department_id_;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
