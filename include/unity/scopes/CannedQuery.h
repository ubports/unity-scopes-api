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

#ifndef UNITY_SCOPES_CANNEDQUERY_H
#define UNITY_SCOPES_CANNEDQUERY_H

#include <unity/util/DefinesPtrs.h>
#include <string>
#include <memory>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace internal
{
class CannedQueryImpl;
}

/**
\brief Parameters of a search query.

Holds all parameters of a search query: the target scope id, query string, department id, and
state of the filters. Can be converted to/from scope:// uri schema string.
*/

class CannedQuery final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(CannedQuery);
    /// @endcond

    /**
    \brief Creates a query for given scope with empty query string.
    */
    explicit CannedQuery(std::string const& scope_id);

    /**
    \brief Creates a query for given scope with specific query string and in given department.
    */
    CannedQuery(std::string const& scope_id, std::string const& query_str, std::string const& department_id);

    /// @cond
    CannedQuery(CannedQuery const &other);
    CannedQuery(CannedQuery&&);
    CannedQuery& operator=(CannedQuery const& other);
    CannedQuery& operator=(CannedQuery&&);
    ~CannedQuery();
    /// @endcond

    /**
    \brief Sets or updates the department.
    */
    void set_department_id(std::string const& dep_id);

    /**
    \brief Sets or updates the query string.
    */
    void set_query_string(std::string const& query_str);

    /**
    \brief Sets filter state.
    */
    void set_filter_state(FilterState const& filter_state);

    /**
    \brief Returns the scope identifier of this CannedQuery.
    \return The scope identifier.
    */
    std::string scope_id() const;

    /**
    \brief Returns the department id of this CannedQuery.
    \return The department id.
    */
    std::string department_id() const;

    /**
    \brief Returns the query string of this CannedQuery.
    \return The query string.
    */
    std::string query_string() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

    /**
    \brief Returns a string representation of this CannedQuery object as a URI using scope:// schema.
    \return The URI for the query.
    */
    std::string to_string() const;

    /**
    \brief Get state of the filters for this CannedQuery.

    Pass this state to methods of specific filter instances (such as
    unity::scopes::OptionSelectorFilter::active_options())to examine filter state.
    \return The state of the filters.
    */
    FilterState filter_state() const;

    /**
    \brief Recreates a CannedQuery object from a scope:// URI.

    \return a CannedQuery instance
    \throws InvalidArgumentException of the URI cannot be parsed.
    */
    static CannedQuery from_string();

private:
    CannedQuery(internal::CannedQueryImpl *impl);
    std::unique_ptr<internal::CannedQueryImpl> p;
    friend class internal::CannedQueryImpl;
};

} // namespace scopes

} // namespace unity

#endif
