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

#ifndef UNITY_SCOPES_QUERY_H
#define UNITY_SCOPES_QUERY_H

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
class QueryImpl;
}

/**
 * \brief Holds all parameters of a search query.

 * Holds all parameters of a search query: the target scope name, query string, department id, and
 * state of the filters. Can be converted to/from scope:// uri schema string.
 */
class Query final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(Query);
    /// @endcond

    /**
     * \brief Creates a query for given scope with empty query string.
     */
    explicit Query(std::string const& scope_name);

    /**
     * \brief Creates a query for given scope with specific query string and in given department.
     */
    Query(std::string const& scope_name, std::string const& query_str, std::string const& department_id);

    /// @cond
    Query(Query const &other);
    Query(Query&&);
    Query& operator=(Query const& other);
    Query& operator=(Query&&);
    ~Query();
    /// @endcond

    /**
     * \brief Sets or updates the department.
     */
    void set_department_id(std::string const& dep_id);

    /**
     * \brief Sets or updates the query string.
     */
    void set_query_string(std::string const& query_str);

    /**
     * \brief Sets filter state.
     */
    void set_filter_state(FilterState const& filter_state);

    /**
     * \brief Returns the scope name of this Query.
     * \return The scope name.
     */
    std::string scope_name() const;

    /**
     * \brief Returns the department id of this Query.
     * \return The department id.
     */
    std::string department_id() const;

    /**
     * \brief Returns the query string of this Query.
     * \return The query string.
     */
    std::string query_string() const;

    /// @cond
    VariantMap serialize() const;
    /// @endcond

    /**
     * \brief Returns a string representation of this Query object as a URI using scope:// schema.
     * \return The URI for the query.
     */
    std::string to_string() const;

    /**
     \brief Get state of the filters for this Query.

     Pass this state to methods of specific filter instances (such as
     unity::scopes::OptionSelectorFilter::active_options())to examine filter state.
     \return The state of the filters.
     */
    FilterState filter_state() const;

    /**
     * \brief Recreates a Query object from a scope:// URI.

     * May throw InvalidArgumentException on invalid schema.
     * \return a Query instance
     * \throws InvalidArgumentException of the URI cannot be parsed.
     */
    static Query from_string();

private:
    Query(internal::QueryImpl *impl);
    std::unique_ptr<internal::QueryImpl> p;
    friend class internal::QueryImpl;
};

} // namespace scopes

} // namespace unity

#endif
