/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Lesser GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Lesser GNU General Public License for more details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_VARIANTVALUECONVERTER_H
#define UNITY_API_SCOPES_INTERNAL_ICEMIDDLEWARE_VARIANTVALUECONVERTER_H

#include <unity/api/scopes/VariantMap.h>
#include <unity/api/scopes/internal/ice_middleware/slice/ValueDict.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace ice_middleware
{

// Utility functions to convert to/from Variant and Value, and to/from VariantMap and ValueDict.

middleware::ValueBasePtr to_value(Variant const& v);
Variant to_variant(middleware::ValueBasePtr const& v);

middleware::ValueDict to_value_dict(VariantMap const& vm);
VariantMap to_variant_map(middleware::ValueDict const& vd);

} // namespace ice_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
