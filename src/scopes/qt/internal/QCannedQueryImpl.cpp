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

#include <unity/scopes/qt/internal/QCannedQueryImpl.h>
#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/scopes/CannedQuery.h>


using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QCannedQueryImpl::QCannedQueryImpl(QString const& scope_id)
    : api_query_(new CannedQuery(scope_id.toUtf8().data()))
{
}

QCannedQueryImpl::QCannedQueryImpl(QString const& scope_id, QString const& query_str, QString const& department_id)
    : api_query_(new CannedQuery(scope_id.toUtf8().data(), query_str.toUtf8().data(), department_id.toUtf8().data()))
{
}

QCannedQueryImpl::QCannedQueryImpl(CannedQuery const& query)
{
    api_query_.reset(new CannedQuery(query));
}

QCannedQueryImpl::QCannedQueryImpl(QCannedQueryImpl const& other)
    : api_query_(new CannedQuery(*other.api_query_))
{
}

void QCannedQueryImpl::set_department_id(QString const& dep_id)
{
    api_query_->set_department_id(dep_id.toUtf8().data());
}

void QCannedQueryImpl::set_query_string(QString const& query_str)
{
    api_query_->set_query_string(query_str.toUtf8().data());
}

void QCannedQueryImpl::set_filter_state(FilterState const& filter_state)
{
    api_query_->set_filter_state(filter_state);
}

QString QCannedQueryImpl::scope_id() const
{
    return QString::fromUtf8(api_query_->scope_id().c_str());
}

QString QCannedQueryImpl::department_id() const
{
    return QString::fromUtf8(api_query_->department_id().c_str());
}

QString QCannedQueryImpl::query_string() const
{
    return QString::fromUtf8(api_query_->query_string().c_str());
}

FilterState QCannedQueryImpl::filter_state() const
{
    return api_query_->filter_state();
}

bool QCannedQueryImpl::has_user_data() const
{
    return api_query_->has_user_data();
}

QVariant QCannedQueryImpl::user_data() const
{
    return variant_to_qvariant(api_query_->user_data());
}

QVariantMap QCannedQueryImpl::serialize() const
{
    return variantmap_to_qvariantmap(api_query_->serialize());
}

QString QCannedQueryImpl::to_uri() const
{
    return QString::fromUtf8(api_query_->to_uri().c_str());
}

QCannedQuery QCannedQueryImpl::from_uri(QString const& uri)
{
    QCannedQueryImpl* internal_query = new QCannedQueryImpl(CannedQuery::from_uri(uri.toUtf8().data()));
    return QCannedQuery(internal_query);
}

QCannedQuery QCannedQueryImpl::create(QCannedQueryImpl* internal)
{
    return QCannedQuery(internal);
}

CannedQuery* QCannedQueryImpl::get_api_query()
{
    return api_query_.get();
}
