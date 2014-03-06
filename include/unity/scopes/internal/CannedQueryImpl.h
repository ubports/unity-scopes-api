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

#ifndef UNITY_SCOPES_INTERNAL_CANNEDQUERYIMPL_H
#define UNITY_SCOPES_INTERNAL_CANNEDQUERYIMPL_H

#include <unity/scopes/Variant.h>
#include <unity/scopes/FilterState.h>
#include <string>

namespace unity
{

namespace scopes
{

class CannedQuery;
class FilterState;

namespace internal
{

class CannedQueryImpl
{
public:
    explicit CannedQueryImpl(std::string const& scope_id);
    CannedQueryImpl(std::string const& scope_id, std::string const& query_str, std::string const& department_id);
    CannedQueryImpl(VariantMap const& variant);
    CannedQueryImpl(CannedQueryImpl const &other) = default;
    CannedQueryImpl(CannedQueryImpl&&) = default;
    CannedQueryImpl& operator=(CannedQueryImpl const& other) = default;
    CannedQueryImpl& operator=(CannedQueryImpl&&) = default;

    void set_department_id(std::string const& dep_id);
    void set_query_string(std::string const& query_str);
    void set_filter_state(FilterState const& filter_state);
    std::string scope_id() const;
    std::string department_id() const;
    std::string query_string() const;
    FilterState filter_state() const;
    VariantMap serialize() const;
    std::string to_string() const;
    static CannedQuery from_string();
    static CannedQuery create(VariantMap const& var);

private:
    std::string scope_id_;
    std::string query_string_;
    std::string department_id_;
    FilterState filter_state_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
