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

#include <unity/UnityExceptions.h>

#include <sstream>

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
{
    other.sync_values();
    api_metadata_.reset(new ActionMetadata(*other.api_metadata_));
}

QActionMetadataImpl::QActionMetadataImpl(QActionMetadataImpl&&) = default;

QActionMetadataImpl& QActionMetadataImpl::operator=(QActionMetadataImpl const& other)
{
    if (&other != this)
    {
        other.sync_values();
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
    sync_values();
    return (scopeVariantMapToQVariantMap(api_metadata_->hints()));
}

bool QActionMetadataImpl::contains_hint(QString const& key) const
{
    sync_values();
    return api_metadata_->contains_hint(key.toUtf8().data());
}

QScopeVariant& QActionMetadataImpl::operator[](QString const& key)
{
    sync_values();
    // look if the key already exists.
    if (ret_variants_.find(key) == ret_variants_.end())
    {
        std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_metadata_)[key.toUtf8().data()])));
        ret_variants_.insert(key, scopeVariant);
    }
    auto it = ret_variants_.find(key);

    unsync_variants_.push_back(it.value());
    return *it.value();
}

QVariant const& QActionMetadataImpl::value(QString const& key) const
{
    sync_values();
    if (ret_variants_.find(key) == ret_variants_.end())
    {
        std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_metadata_)[key.toUtf8().data()])));
        ret_variants_.insert(key, scopeVariant);
    }
    std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_metadata_)[key.toUtf8().data()])));
    ret_variants_.insert(key, scopeVariant);

    return *ret_variants_.find(key).value();
}

void QActionMetadataImpl::sync_values() const
{
    QListIterator<std::shared_ptr<QScopeVariant>> it(unsync_variants_);
    while (it.hasNext())
    {
        it.next()->sync();
    }
    unsync_variants_.clear();
}
