/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#if !defined(UNITY_SCOPES_INTERNAL_LTTNG_UNITYSCOPESTP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define UNITY_SCOPES_INTERNAL_LTTNG_UNITYSCOPESTP_H

#undef TRACEPOINT_PROVIDER
#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_PROVIDER unity_scopes_api
#define TRACEPOINT_INCLUDE <scopes/internal/lttng/UnityScopes_tp.h>

#include <scopes/internal/lttng/SimpleTracepoint.h>
#include <stdint.h>

SIMPLE_TRACEPOINT(
  trace,
  TRACE_DEBUG,
  stp_integer(int, value)
)

#endif /* UNITY_API_SCOPES_INTERNAL_LTTNG_UNITYSCOPESAPI_TP_H */
