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

#include <core/signal.h>
#include <unity/util/DefinesPtrs.h>
#include <unity/util/NonCopyable.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class MWSubscriber
{
public:
    NONCOPYABLE(MWSubscriber);
    UNITY_DEFINES_PTRS(MWSubscriber);

    virtual ~MWSubscriber();

    virtual std::string endpoint() const = 0;
    core::Signal<std::string const&> const& message_received() const;

protected:
    MWSubscriber();

    core::Signal<std::string const&> message_received_;
};

} // namespace internal

} // namespace scopes

} // namespace unity
