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

#ifndef UNITY_API_SCOPES_INTERNAL_EVENTLOGGER_H
#define UNITY_API_SCOPES_INTERNAL_EVENTLOGGER_H

#include <stdint.h>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class EventLogger
{
public:
  virtual ~EventLogger() = default;

  virtual void received_event(int64_t when, int type, float code, const char* value) = 0;
  virtual void received_event2(int src_fd, uint32_t seq_id) = 0;

protected:
  EventLogger() = default;
  EventLogger(EventLogger const&) = delete;
  EventLogger& operator=(EventLogger const&) = delete;
};

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif // UNITY_API_SCOPES_INTERNAL_EVENTLOGGER_H
