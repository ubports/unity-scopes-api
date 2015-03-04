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
#include <unity/scopes/qt/internal/QScopeVariant.h>
#include <unity/scopes/qt/internal/QUtils.h>

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
{
    other.sync_values();
    api_search_metadata_.reset(new SearchMetadata(*other.api_search_metadata_));
}

QSearchMetadataImpl::QSearchMetadataImpl(QSearchMetadataImpl&&) = default;
QSearchMetadataImpl& QSearchMetadataImpl::operator=(QSearchMetadataImpl&&) = default;
QSearchMetadataImpl::~QSearchMetadataImpl() = default;

QSearchMetadataImpl& QSearchMetadataImpl::operator=(QSearchMetadataImpl const& other)
{
    if (&other != this)
    {
        other.sync_values();
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
    sync_values();
    return api_search_metadata_->cardinality();
}

void QSearchMetadataImpl::set_location(Location const& location)
{
    api_search_metadata_->set_location(location);
}

Location QSearchMetadataImpl::location() const
{
    sync_values();
    return api_search_metadata_->location();
}

bool QSearchMetadataImpl::has_location() const
{
    sync_values();
    return api_search_metadata_->has_location();
}

void QSearchMetadataImpl::remove_location()
{
    sync_values();
    api_search_metadata_->remove_location();
}

void QSearchMetadataImpl::set_hint(QString const& key, QVariant const& value)
{
    api_search_metadata_->set_hint(key.toUtf8().data(), qvariant_to_variant(value));
}

QVariantMap QSearchMetadataImpl::hints() const
{
    sync_values();
    return variantmap_to_qvariantmap(api_search_metadata_->hints());
}

bool QSearchMetadataImpl::contains_hint(QString const& key) const
{
    return api_search_metadata_->contains_hint(key.toUtf8().data());
}

QScopeVariant& QSearchMetadataImpl::operator[](QString const& key)
{
    sync_values();
    // look if the key already exists.
    if (ret_variants_.find(key) == ret_variants_.end())
    {
        std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_search_metadata_)[key.toUtf8().data()])));
        ret_variants_.insert(key, scopeVariant);
    }
    auto it = ret_variants_.find(key);

    unsync_variants_.push_back(it.value());
    return *it.value();
}

QVariant const& QSearchMetadataImpl::value(QString const& key) const
{
    sync_values();
    if (ret_variants_.find(key) == ret_variants_.end())
    {
        std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_search_metadata_)[key.toUtf8().data()])));
        ret_variants_.insert(key, scopeVariant);
    }
    std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_search_metadata_)[key.toUtf8().data()])));
    ret_variants_.insert(key, scopeVariant);

    return *ret_variants_.find(key).value();
}

void QSearchMetadataImpl::sync_values() const
{
    QListIterator<std::shared_ptr<QScopeVariant>> it(unsync_variants_);
    while (it.hasNext())
    {
        it.next()->sync();
    }
    unsync_variants_.clear();
}
