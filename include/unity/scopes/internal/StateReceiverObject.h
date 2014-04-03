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

#ifndef UNITY_SCOPES_INTERNAL_STATERECEIVEROBJECT_H
#define UNITY_SCOPES_INTERNAL_STATERECEIVEROBJECT_H

#include <unity/scopes/internal/AbstractObject.h>
#include <unity/scopes/internal/InvokeInfo.h>
#include <unity/util/DefinesPtrs.h>

#include <core/signal.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class StateReceiverObject : public AbstractObject
{
public:
    UNITY_DEFINES_PTRS(StateReceiverObject);

    enum State
    {
        ScopeReady,
        ScopeStopping
    };

    StateReceiverObject();
    virtual ~StateReceiverObject();

    // Remote operation implementations
    void push_state(std::string const& sender_id, State const& state);

    // Local methods
    core::Signal<std::string, State> const& state_received() const;

private:
    core::Signal<std::string, State> state_received_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif // UNITY_SCOPES_INTERNAL_STATERECEIVEROBJECT_H
