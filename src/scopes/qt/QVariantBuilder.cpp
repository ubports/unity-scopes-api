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

#include <unity/scopes/qt/QVariantBuilder.h>

#include <unity/scopes/qt/internal/QVariantBuilderImpl.h>

#include <unity/scopes/VariantBuilder.h>

using namespace unity::scopes::qt;

QVariantBuilder::QVariantBuilder()
    : p(new internal::QVariantBuilderImpl())
{
}

QVariantBuilder::~QVariantBuilder() = default;

QVariantBuilder::QVariantBuilder(QVariantBuilder const& other)
    : p(new internal::QVariantBuilderImpl(*other.p))
{
}

QVariantBuilder::QVariantBuilder(QVariantBuilder&&) = default;

QVariantBuilder& QVariantBuilder::operator=(QVariantBuilder const& other)
{
    if (&other != this)
    {
        p.reset(new internal::QVariantBuilderImpl(*other.p));
    }

    return *this;
}

QVariantBuilder& QVariantBuilder::operator=(QVariantBuilder&& other) = default;

void QVariantBuilder::add_tuple(QVector<QPair<QString, QVariant>> const& tuple)
{
    p->add_tuple(tuple);
}

QVariant QVariantBuilder::end()
{
    return p->end();
}
