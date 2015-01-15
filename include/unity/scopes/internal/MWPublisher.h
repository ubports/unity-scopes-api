/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#pragma once

#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

#include <string>

namespace unity
{

namespace scopes
{

namespace internal
{

class MWPublisher
{
public:
    NONCOPYABLE(MWPublisher);
    UNITY_DEFINES_PTRS(MWPublisher);

    virtual ~MWPublisher();

    virtual std::string endpoint() const = 0;
    virtual void send_message(std::string const& message, std::string const& topic = "") = 0;

protected:
    MWPublisher();
};

} // namespace internal

} // namespace scopes

} // namespace unity
