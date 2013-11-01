/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_RETHROWEXCEPTION_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_RETHROWEXCEPTION_H

#include <zmqpp/exception.hpp>

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

// Convert a zmq exception to a unity::MiddlewareException and throw it.

void rethrow_zmq_ex(zmqpp::exception const& e);

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
