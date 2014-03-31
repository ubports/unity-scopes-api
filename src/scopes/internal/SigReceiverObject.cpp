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

#include <unity/scopes/internal/SigReceiverObject.h>

namespace unity
{

namespace scopes
{

namespace internal
{

SigReceiverObject::SigReceiverObject()
{
}

SigReceiverObject::~SigReceiverObject()
{
}

void SigReceiverObject::push_signal(SignalType const& signal, const InvokeInfo &info)
{
    signal_received_(info.id, signal);
}

core::Signal<std::string, SigReceiverObject::SignalType> const& SigReceiverObject::signal_received() const
{
    return signal_received_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
