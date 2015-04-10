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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariantMap>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

class ActionMetadata;

namespace qt
{

class QPreviewQueryBaseAPI;

namespace internal
{
class QScopeVariant;

class QPreviewQueryBaseImpl;

class QActionMetadataImpl
{
public:
    UNITY_DEFINES_PTRS(QActionMetadataImpl);

    QActionMetadataImpl(QString const& locale, QString const& form_factor);
    ~QActionMetadataImpl();

    QActionMetadataImpl(QActionMetadataImpl const& other);
    QActionMetadataImpl(QActionMetadataImpl&&);

    QActionMetadataImpl& operator=(QActionMetadataImpl const& other);
    QActionMetadataImpl& operator=(QActionMetadataImpl&&);

    void set_scope_data(QVariant const& data);
    QVariant scope_data() const;
    void set_hint(QString const& key, QVariant const& value);
    QVariantMap hints() const;
    bool contains_hint(QString const& key) const;
    QScopeVariant& operator[](QString const& key);
    QVariant const& value(QString const& key) const;

private:
    QActionMetadataImpl(ActionMetadata const& api_metadata);

    void sync_values() const;

    std::unique_ptr<ActionMetadata> api_metadata_;

    mutable QMap<QString, std::shared_ptr<QScopeVariant>> ret_variants_;
    mutable QList<std::shared_ptr<QScopeVariant>> unsync_variants_;

    friend class internal::QPreviewQueryBaseImpl;
    friend class unity::scopes::qt::QPreviewQueryBaseAPI;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity
