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

#ifndef _ENABLE_QT_EXPERIMENTAL_
#error You should define _ENABLE_QT_EXPERIMENTAL_ in order to use this experimental header file.
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <QtCore/QVariant>
#pragma GCC diagnostic pop
#include <unity/scopes/Variant.h>

namespace unity
{

namespace scopes
{

namespace qt
{

namespace internal
{

QVariant variant_to_qvariant(unity::scopes::Variant const& variant);
unity::scopes::Variant qvariant_to_variant(QVariant const& variant);
QVariantMap variantmap_to_qvariantmap(unity::scopes::VariantMap const& variant);
VariantMap qvariantmap_to_variantmap(QVariantMap const& variant);

}  // namespace qt

}  // namespace qt

}  // namespace scopes

}  // namespace unity
