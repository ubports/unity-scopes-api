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

#include <unity/scopes/qt/internal/QScopeVariant.h>
#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/scopes/Variant.h>

#include <QtCore/QMap>

#include <cassert>

using namespace std;
using namespace unity::scopes::qt::internal;

/// @cond
// QScopeVariant::QScopeVariant() noexcept : QVariant(), internal_variant_(nullptr)
//{
//}

QScopeVariant::QScopeVariant(Variant* val) noexcept : QVariant(variant_to_qvariant(*val)), internal_variant_(val)
{
}

QScopeVariant::~QScopeVariant() = default;

QScopeVariant::QScopeVariant(QScopeVariant const& other)
    : QVariant(variant_to_qvariant(*other.internal_variant_))
    , internal_variant_(other.internal_variant_)
{
}

QScopeVariant::QScopeVariant(QScopeVariant&&) = default;

QScopeVariant& QScopeVariant::operator=(QScopeVariant const& other)
{
    if (&other != this)
    {
        internal_variant_ = other.internal_variant_;
    }

    return *this;
}

QScopeVariant& QScopeVariant::operator=(QScopeVariant&&) = default;

void QScopeVariant::setInternalVariant(Variant* val)
{
    internal_variant_ = val;
}

QScopeVariant& QScopeVariant::operator=(int val) noexcept
{
    assert(internal_variant_);
    *internal_variant_ = val;
    return *this;
}

QScopeVariant& QScopeVariant::operator=(double val) noexcept
{
    assert(internal_variant_);
    *internal_variant_ = val;
    return *this;
}

QScopeVariant& QScopeVariant::operator=(bool val) noexcept
{
    assert(internal_variant_);
    *internal_variant_ = val;
    return *this;
}

QScopeVariant& QScopeVariant::operator=(QString const& val)
{
    assert(internal_variant_);
    *internal_variant_ = val.toUtf8().data();
    return *this;
}

QScopeVariant& QScopeVariant::operator=(char const* val)
{
    assert(internal_variant_);
    *internal_variant_ = val;
    return *this;
}

QScopeVariant& QScopeVariant::operator=(QVariantMap const& val)
{
    assert(internal_variant_);
    VariantMap internal_val;

    QMapIterator<QString, QVariant> it(val);
    while (it.hasNext())
    {
        internal_val[it.key().toUtf8().data()] = qvariant_to_variant(it.value());
    }
    *internal_variant_ = internal_val;
    return *this;
}

QScopeVariant& QScopeVariant::operator=(QVariant const& val)
{
    Variant internal_val = qvariant_to_variant(val);
    *internal_variant_ = internal_val;
    return *this;
}

std::string QScopeVariant::get_string() const
{
    assert(internal_variant_);
    return internal_variant_->get_string();
}

void QScopeVariant::sync()
{
    *internal_variant_ = qvariant_to_variant(*this);
}
/// @endcond
