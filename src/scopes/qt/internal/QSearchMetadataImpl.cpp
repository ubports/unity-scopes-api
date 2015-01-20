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

#include <unity/scopes/qt/internal/QSearchMetadataImpl.h>

#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/SearchMetadata.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

QSearchMetadataImpl::QSearchMetadataImpl(QString const& locale, QString const& form_factor)
    : api_search_metadata_(new SearchMetadata(locale.toUtf8().data(), form_factor.toUtf8().data()))
{
}

QSearchMetadataImpl::QSearchMetadataImpl(int cardinality, QString const& locale, QString const& form_factor)
    : api_search_metadata_(new SearchMetadata(cardinality, locale.toUtf8().data(), form_factor.toUtf8().data()))
{
}

QSearchMetadataImpl::QSearchMetadataImpl(QSearchMetadataImpl const& other)
    : api_search_metadata_(new SearchMetadata(*other.api_search_metadata_))
{
}

QSearchMetadataImpl::QSearchMetadataImpl(QSearchMetadataImpl&&) = default;
QSearchMetadataImpl& QSearchMetadataImpl::operator=(QSearchMetadataImpl&&) = default;
QSearchMetadataImpl::~QSearchMetadataImpl() = default;

QSearchMetadataImpl& QSearchMetadataImpl::operator=(QSearchMetadataImpl const& other)
{
    if (&other != this)
    {
        api_search_metadata_.reset(new SearchMetadata(*other.api_search_metadata_));
    }
    return *this;
}

void QSearchMetadataImpl::set_cardinality(int cardinality)
{
    api_search_metadata_->set_cardinality(cardinality);
}

int QSearchMetadataImpl::cardinality() const
{
    return api_search_metadata_->cardinality();
}

void QSearchMetadataImpl::set_location(Location const& location)
{
    api_search_metadata_->set_location(location);
}

Location QSearchMetadataImpl::location() const
{
    return api_search_metadata_->location();
}

bool QSearchMetadataImpl::has_location() const
{
    return api_search_metadata_->cardinality();
}

void QSearchMetadataImpl::set_hint(QString const& key, QVariant const& value)
{
    api_search_metadata_->set_hint(key.toUtf8().data(), qVariantToScopeVariant(value));
}

QVariantMap QSearchMetadataImpl::hints() const
{
    return scopeVariantMapToQVariantMap(api_search_metadata_->hints());
}

bool QSearchMetadataImpl::contains_hint(QString const& key) const
{
    return api_search_metadata_->contains_hint(key.toUtf8().data());
}

QScopeVariant& QSearchMetadataImpl::operator[](QString const& key)
{
    // we maintain an internal map of values to keep valid references to
    // the internal api's Variants
    QScopeVariant qVariant(&((*api_search_metadata_)[key.toUtf8().data()]));
    return_variants[key] = qVariant;
    return return_variants[key];
}

QVariant const& QSearchMetadataImpl::value(QString const& key) const
{
    // we maintain an internal map of values to keep valid references to
    // the internal api's Variants
    return_const_variants[key] = scopeVariantToQVariant((*api_search_metadata_)[key.toUtf8().data()]);
    return return_const_variants[key];
}
