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

#include <unity/scopes/qt/QActionMetadata.h>
#include <unity/scopes/qt/QScopeVariant.h>

#include <unity/scopes/qt/internal/QActionMetadataImpl.h>

using namespace unity::scopes::qt;

/// @cond
QActionMetadata::~QActionMetadata() = default;

QActionMetadata::QActionMetadata(QString const& locale, QString const& form_factor)
    : p(new internal::QActionMetadataImpl(locale, form_factor))
{
}

QActionMetadata::QActionMetadata(internal::QActionMetadataImpl* impl)
    : p(new internal::QActionMetadataImpl(*impl))
{
}

void QActionMetadata::set_scope_data(QVariant const& data)
{
    p->set_scope_data(data);
}

QVariant QActionMetadata::scope_data() const
{
    return p->scope_data();
}

QActionMetadata::QActionMetadata(QActionMetadata const& other)
    : p(new internal::QActionMetadataImpl(*other.p))
{
}

QActionMetadata::QActionMetadata(QActionMetadata&&) = default;

QActionMetadata& QActionMetadata::operator=(QActionMetadata const& other)
{
    if (&other != this)
    {
        p.reset(new internal::QActionMetadataImpl(*other.p));
    }

    return *this;
}

QActionMetadata& QActionMetadata::operator=(QActionMetadata &&) = default;

void QActionMetadata::set_hint(QString const& key, QVariant const& value)
{
    p->set_hint(key, value);
}

QVariantMap QActionMetadata::hints() const
{
    return p->hints();
}

bool QActionMetadata::contains_hint(QString const& key) const
{
    return p->contains_hint(key);
}

QVariant& QActionMetadata::operator[](QString const& key)
{
    return (*p)[key];
}

QVariant const& QActionMetadata::operator[](QString const& key) const
{
    return p->value(key);
}

/// @endcond
