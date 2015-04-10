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
#include <unity/scopes/Location.h>

#include <QtCore/QString>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariant>
#pragma GCC diagnostic pop

namespace unity
{

namespace scopes
{

class SearchMetadata;

namespace qt
{

namespace internal
{

class QScopeVariant;
class QSearchMetadataImpl
{
public:
    UNITY_DEFINES_PTRS(QSearchMetadataImpl);

    QSearchMetadataImpl(QString const& locale, QString const& form_factor);
    QSearchMetadataImpl(int cardinality, QString const& locale, QString const& form_factor);
    ~QSearchMetadataImpl();

    QSearchMetadataImpl(QSearchMetadataImpl const& other);
    QSearchMetadataImpl(QSearchMetadataImpl&&);

    QSearchMetadataImpl& operator=(QSearchMetadataImpl const& other);
    QSearchMetadataImpl& operator=(QSearchMetadataImpl&&);

    void set_cardinality(int cardinality);
    int cardinality() const;
    void set_location(Location const& location);
    Location location() const;
    bool has_location() const;
    void remove_location();
    void set_hint(QString const& key, QVariant const& value);
    QVariantMap hints() const;
    bool contains_hint(QString const& key) const;
    QScopeVariant& operator[](QString const& key);
    QVariant const& value(QString const& key) const;

private:
    void sync_values() const;

    mutable QMap<QString, std::shared_ptr<QScopeVariant>> ret_variants_;
    mutable QList<std::shared_ptr<QScopeVariant>> unsync_variants_;

    std::unique_ptr<unity::scopes::SearchMetadata> api_search_metadata_;
};

}  // namespace internal

}  // namespace qt

}  // namespace scopes

}  // namespace unity
