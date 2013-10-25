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

#define TRACEPOINT_DEFINE

#include <unity/api/scopes/internal/lttng/LttngLogger.h>
#include <unity/api/scopes/internal/lttng/LttngLogger_tp.h>

void unity::api::scopes::internal::lttng::LttngLogger::received_event(int64_t when, int type, float code, const char *value)
{
  simple_tracepoint(unity_scopes_api, received_event, when, type, code, value);
}

void unity::api::scopes::internal::lttng::LttngLogger::received_event2(int src_fd, uint32_t seq_id)
{
  simple_tracepoint(unity_scopes_api, received_event2, src_fd, seq_id);
}
