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

#include <unity/scopes/qt/QSearchMetadata.h>
#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/qt/internal/QSearchMetadataImpl.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;

/// @cond
QSearchMetadata::QSearchMetadata(QString const& locale, QString const& form_factor)
    : p(new internal::QSearchMetadataImpl(locale, form_factor))
{
}

QSearchMetadata::QSearchMetadata(int cardinality, QString const& locale, QString const& form_factor)
    : p(new internal::QSearchMetadataImpl(cardinality, locale, form_factor))
{
}

QSearchMetadata::QSearchMetadata(QSearchMetadata const& other)
    : p(new internal::QSearchMetadataImpl(*other.p))
{
}

QSearchMetadata::QSearchMetadata(QSearchMetadata&&) = default;

QSearchMetadata::~QSearchMetadata() = default;

void QSearchMetadata::set_cardinality(int cardinality)
{
    p->set_cardinality(cardinality);
}

int QSearchMetadata::cardinality() const
{
    return p->cardinality();
}

void QSearchMetadata::set_location(Location const& location)
{
    p->set_location(location);
}

Location QSearchMetadata::location() const
{
    return p->location();
}

bool QSearchMetadata::has_location() const
{
    return p->has_location();
}

void QSearchMetadata::set_hint(QString const& key, QVariant const& value)
{
    p->set_hint(key, value);
}

QVariantMap QSearchMetadata::hints() const
{
    return p->hints();
}

bool QSearchMetadata::contains_hint(QString const& key) const
{
    return p->contains_hint(key);
}

QVariant& QSearchMetadata::operator[](QString const& key)
{
    return (*p)[key];
}

QVariant const& QSearchMetadata::operator[](QString const& key) const
{
    return p->value(key);
}
/// @endcond
