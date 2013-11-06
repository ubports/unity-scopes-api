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

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_VARIANTCONVERTER_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_VARIANTCONVERTER_H

#include <unity/api/scopes/Variant.h>
#include <unity/api/scopes/internal/zmq_middleware/capnproto/ValueDict.capnp.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

// Utility functions to convert to/from Variant and VariantMap.

void to_value(Variant const& v, unity::api::scopes::internal::zmq_middleware::capnproto::Value::Builder& b);
Variant to_variant(unity::api::scopes::internal::zmq_middleware::capnproto::Value::Reader const& r);

void to_value_dict(VariantMap const& vm, unity::api::scopes::internal::zmq_middleware::capnproto::ValueDict::Builder& b);
VariantMap to_variant_map(unity::api::scopes::internal::zmq_middleware::capnproto::ValueDict::Reader const& r);

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
