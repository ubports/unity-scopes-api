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

#include <unity/scopes/qt/internal/QActionMetadataImpl.h>

#include <unity/scopes/qt/QUtils.h>
#include <unity/scopes/qt/QScopeVariant.h>

#include <unity/scopes/ActionMetadata.h>

using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QActionMetadataImpl::~QActionMetadataImpl() = default;

QActionMetadataImpl::QActionMetadataImpl(QString const& locale, QString const& form_factor)
    : api_metadata_(new ActionMetadata(locale.toUtf8().data(), form_factor.toUtf8().data()))
{
}

QActionMetadataImpl::QActionMetadataImpl(ActionMetadata const& api_metadata)
    : api_metadata_(new ActionMetadata(api_metadata))
{
}

void QActionMetadataImpl::set_scope_data(QVariant const& data)
{
    api_metadata_->set_scope_data(qVariantToScopeVariant(data));
}

QVariant QActionMetadataImpl::scope_data() const
{
    return scopeVariantToQVariant(api_metadata_->scope_data());
}

QActionMetadataImpl::QActionMetadataImpl(QActionMetadataImpl const& other)
    : api_metadata_(new ActionMetadata(*other.api_metadata_))
{
}

QActionMetadataImpl::QActionMetadataImpl(QActionMetadataImpl&&) = default;

QActionMetadataImpl& QActionMetadataImpl::operator=(QActionMetadataImpl const& other)
{
    if (&other != this)
    {
        api_metadata_.reset(new ActionMetadata(*other.api_metadata_));
    }

    return *this;
}

QActionMetadataImpl& QActionMetadataImpl::operator=(QActionMetadataImpl&&) = default;

void QActionMetadataImpl::set_hint(QString const& key, QVariant const& value)
{
    api_metadata_->set_hint(key.toUtf8().data(), qVariantToScopeVariant(value));
}

QVariantMap QActionMetadataImpl::hints() const
{
    return (scopeVariantMapToQVariantMap(api_metadata_->hints()));
}

bool QActionMetadataImpl::contains_hint(QString const& key) const
{
    return api_metadata_->contains_hint(key.toUtf8().data());
}

QScopeVariant& QActionMetadataImpl::operator[](QString const& key)
{
    QScopeVariant qVariant(&((*api_metadata_)[key.toUtf8().data()]));
    return_variants[key] = qVariant;
    return return_variants[key];
}

QVariant const& QActionMetadataImpl::value(QString const& key) const
{
    return_const_variants[key] = scopeVariantToQVariant((*api_metadata_)[key.toUtf8().data()]);
    return return_const_variants[key];
}
