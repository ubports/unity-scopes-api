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

#include <unity/scopes/qt/QCannedQuery.h>
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

class CategorisedResult;

namespace internal
{
class ResultImpl;
}

namespace qt
{

namespace internal
{

class QDepartmentImpl;

class QCannedQueryImpl
{
public:
    explicit QCannedQueryImpl(QString const& scope_id);
    QCannedQueryImpl(QString const& scope_id, QString const& query_str, QString const& department_id);
    QCannedQueryImpl(CannedQuery const& query);
    QCannedQueryImpl(QCannedQueryImpl const& other);
    QCannedQueryImpl(QCannedQueryImpl&&) = default;
    QCannedQueryImpl& operator=(QCannedQueryImpl const& other) = default;
    QCannedQueryImpl& operator=(QCannedQueryImpl&&) = default;

    void set_department_id(QString const& dep_id);
    void set_query_string(QString const& query_str);
    void set_filter_state(FilterState const& filter_state);
    QString scope_id() const;
    QString department_id() const;
    QString query_string() const;
    FilterState filter_state() const;
    QVariantMap serialize() const;
    QString to_uri() const;
    bool has_user_data() const;
    QVariant user_data() const;
    static QCannedQuery from_uri(QString const& uri);

    // added for testing purposes
    static QCannedQuery create(QCannedQueryImpl* internal);
    // added for testing purposes
    CannedQuery* get_api_query();

private:
    std::unique_ptr<CannedQuery> api_query_;
    friend class QDepartmentImpl;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity
