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

#ifndef UNITY_API_SCOPES_QUERY_H
#define UNITY_API_SCOPES_QUERY_H

#include <unity/SymbolExport.h>
#include <string>
#include <memory>
#include <scopes/FilterState.h>
#include <scopes/Variant.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{
class QueryImpl;
}

class UNITY_API Query final
{
public:
    Query(std::string const& scope_name);
    Query(std::string const& scope_name, std::string const& query_str, std::string const& department_id);
    Query(Query const &other);
    Query(Query&&);
    Query& operator=(Query const& other);
    Query& operator=(Query&&);

    void set_department_id(std::string const& dep_id);
    void set_query_string(std::string const& query_str);
    void set_filter_state(FilterState const& filter_state);
    std::string scope_name() const;
    std::string department_id() const;
    std::string query_string() const;

    VariantMap serialize() const;
    std::string to_string() const;
    FilterState filter_state() const;
    static Query from_string();

private:
    std::shared_ptr<internal::QueryImpl> p;
};

} // namespace scopes

} // namespace api

} // namespace unity

#endif
