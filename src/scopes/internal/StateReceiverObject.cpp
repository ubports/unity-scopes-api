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

#include <unity/scopes/internal/StateReceiverObject.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

StateReceiverObject::StateReceiverObject()
{
}

StateReceiverObject::~StateReceiverObject()
{
}

void StateReceiverObject::push_state(std::string const& sender_id, State const& state)
{
    lock_guard<mutex> lock(mutex_);
    state_received_(sender_id, state);
}

core::Signal<std::string, StateReceiverObject::State> const& StateReceiverObject::state_received() const
{
    lock_guard<mutex> lock(mutex_);
    return state_received_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
