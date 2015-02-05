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

#include <unity/scopes/qt/QCannedQuery.h>

#include <unity/scopes/qt/internal/QCannedQueryImpl.h>

#include <unity/scopes/CannedQuery.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;

QCannedQuery::QCannedQuery(QString const& scope_id)
    : p(new internal::QCannedQueryImpl(scope_id))
{
}

QCannedQuery::QCannedQuery(QString const& scope_id, QString const& query_str, QString const& department_id)
    : p(new internal::QCannedQueryImpl(scope_id, query_str, department_id))
{
}

/// @cond
QCannedQuery::QCannedQuery(QCannedQuery const& other)
    : p(new internal::QCannedQueryImpl(*(other.p)))
{
}

QCannedQuery::QCannedQuery(internal::QCannedQueryImpl* impl)
    : p(impl)
{
}

QCannedQuery::QCannedQuery(CannedQuery const& api_query)
    : p(new internal::QCannedQueryImpl(api_query))
{
}

QCannedQuery::QCannedQuery(QCannedQuery&&) = default;

QCannedQuery::~QCannedQuery() = default;

QCannedQuery& QCannedQuery::operator=(QCannedQuery const& other)
{
    if (this != &other)
    {
        p.reset(new internal::QCannedQueryImpl(*(other.p)));
    }
    return *this;
}

QCannedQuery& QCannedQuery::operator=(QCannedQuery&&) = default;

QVariantMap QCannedQuery::serialize() const
{
    return p->serialize();
}
/// @endcond

void QCannedQuery::set_department_id(QString const& dep_id)
{
    p->set_department_id(dep_id);
}

void QCannedQuery::set_query_string(QString const& query_str)
{
    p->set_query_string(query_str);
}

void QCannedQuery::set_filter_state(FilterState const& filter_state)
{
    p->set_filter_state(filter_state);
}

QString QCannedQuery::scope_id() const
{
    return p->scope_id();
}

QString QCannedQuery::department_id() const
{
    return p->department_id();
}

QString QCannedQuery::query_string() const
{
    return p->query_string();
}

FilterState QCannedQuery::filter_state() const
{
    return p->filter_state();
}

QString QCannedQuery::to_uri() const
{
    return p->to_uri();
}

QCannedQuery QCannedQuery::from_uri(QString const& uri)
{
    return internal::QCannedQueryImpl::from_uri(uri);
}
