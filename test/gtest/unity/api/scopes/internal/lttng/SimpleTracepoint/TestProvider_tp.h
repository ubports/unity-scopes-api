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

#if !defined(SIMPLETRACEPOINTTEST_TESTPROVIDER_TP_H) || defined(TRACEPOINT_HEADER_MULTI_READ)
#define SIMPLETRACEPOINTTEST_TESTPROVIDER_TP_H

#undef TRACEPOINT_PROVIDER
#undef TRACEPOINT_INCLUDE
#define TRACEPOINT_PROVIDER test_provider
#define TRACEPOINT_INCLUDE <TestProvider_tp.h>

#include <scopes/internal/lttng/SimpleTracepoint.h>
#include <stdint.h>

SIMPLE_TRACEPOINT(
  event1,
  TRACE_DEBUG,
  stp_integer(int64_t, a),
  stp_integer(int, b),
  stp_float(float, c),
  stp_string(d)
)

SIMPLE_TRACEPOINT(
  event2,
  TRACE_DEBUG,
  stp_float(double, a),
  stp_integer(uint32_t, b)
)

#endif /* SIMPLETRACEPOINTTEST_TESTPROVIDER_TP_H */
