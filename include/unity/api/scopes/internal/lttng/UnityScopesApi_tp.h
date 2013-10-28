/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#if !defined(UNITY_API_SCOPES_INTERNAL_LTTNG_EVENTLOGGER_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define UNITY_API_SCOPES_INTERNAL_LTTNG_EVENTLOGGER_TP_H

#undef TRACEPOINT_PROVIDER
#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_PROVIDER unity_scopes_api
#define TRACEPOINT_INCLUDE <unity/api/scopes/internal/lttng/UnityScopesApi_tp.h>

#include <unity/api/scopes/internal/lttng/simple_tracepoint.h>
#include <stdint.h>

SIMPLE_TRACEPOINT(
  received_event,
  TRACE_DEBUG,
  stp_integer(int64_t, when),
  stp_integer(int, type),
  stp_float(float, code),
  stp_string(value)
)

SIMPLE_TRACEPOINT(
  received_event2,
  TRACE_DEBUG,
  stp_integer(int, src_fd),
  stp_integer(uint32_t, seq_id)
)

#endif /* UNITY_API_SCOPES_INTERNAL_LTTNG_EVENTLOGGER_TP_H */
