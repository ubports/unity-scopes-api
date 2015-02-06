/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma once

#include <unity/util/DefinesPtrs.h>
#include <unity/scopes/FilterState.h>

#include <QtCore/QString>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariantMap>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

class CannedQuery;

namespace qt
{

namespace internal
{
class QCannedQueryImpl;
class QSearchQueryBaseImpl;
class QDepartmentImpl;
}

/**
\brief Parameters of a search query.

Holds all parameters of a search query: the target scope id, query string, department id, and
state of the filters. Can be converted to/from scope:// uri schema string.
*/
class QCannedQuery final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QCannedQuery);
    /// @endcond

    /**
    \brief Creates a query for given scope with empty query string.
    */
    explicit QCannedQuery(QString const& scope_id);

    /**
    \brief Creates a query for given scope with specific query string and in given department.
    */
    QCannedQuery(QString const& scope_id, QString const& query_str, QString const& department_id);

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QCannedQuery(QCannedQuery const& other);
    QCannedQuery(QCannedQuery&&);
    QCannedQuery& operator=(QCannedQuery const& other);
    QCannedQuery& operator=(QCannedQuery&&);
    //@}

    /// @cond
    ~QCannedQuery();
    /// @endcond

    /**
    \brief Sets or updates the department.
    */
    void set_department_id(QString const& dep_id);

    /**
    \brief Sets or updates the query string.
    */
    void set_query_string(QString const& query_str);

    /**
    \brief Sets filter state.
    */
    void set_filter_state(FilterState const& filter_state);

    /**
    \brief Returns the scope identifier of this CannedQuery.
    \return The scope identifier.
    */
    QString scope_id() const;

    /**
    \brief Returns the department id of this CannedQuery.
    \return The department id.
    */
    QString department_id() const;

    /**
    \brief Returns the query string of this CannedQuery.
    \return The query string.
    */
    QString query_string() const;

    /// @cond
    QVariantMap serialize() const;
    /// @endcond

    /**
    \brief Returns a string representation of this CannedQuery object as a URI using scope:// schema.
    \return The URI for the query.
    */
    QString to_uri() const;

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
    \throws InvalidArgumentException if the URI cannot be parsed.
    */
    static QCannedQuery from_uri(QString const& uri);

private:
    QCannedQuery(internal::QCannedQueryImpl* impl);
    QCannedQuery(CannedQuery const& api_query);
    std::unique_ptr<internal::QCannedQueryImpl> p;
    friend class internal::QCannedQueryImpl;
    friend class internal::QSearchQueryBaseImpl;
    friend class internal::QDepartmentImpl;
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity
