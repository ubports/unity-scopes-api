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

#include <unity/scopes/qt/QColumnLayout.h>

#include <unity/scopes/qt/internal/QColumnLayoutImpl.h>

using namespace unity::scopes::qt;

/// @cond
QColumnLayout::QColumnLayout(int num_of_columns)
    : p(new internal::QColumnLayoutImpl(num_of_columns))
{
}

QColumnLayout::QColumnLayout(QColumnLayout const& other)
    : p(new internal::QColumnLayoutImpl(other.p->api_layout_.get()))
{
}

QColumnLayout::QColumnLayout(QColumnLayout&&) = default;

QColumnLayout& QColumnLayout::operator=(QColumnLayout const& other)
{
    if (&other != this)
    {
        p.reset(new internal::QColumnLayoutImpl(other.p->api_layout_.get()));
    }
    return *this;
}

QColumnLayout::QColumnLayout(internal::QColumnLayoutImpl * impl) : p(impl)
{
}

QColumnLayout& QColumnLayout::operator=(QColumnLayout&&) = default;

QColumnLayout::~QColumnLayout() = default;

void QColumnLayout::add_column(QVector<QString> widget_ids)
{
    p->add_column(widget_ids);
}

int QColumnLayout::size() const noexcept
{
    return p->size();
}

int QColumnLayout::number_of_columns() const noexcept
{
    return p->number_of_columns();
}

QVector<QString> QColumnLayout::column(int index) const
{
    return p->column(index);
}

QVariantMap QColumnLayout::serialize() const
{
    return p->serialize();
}
/// @endcond
