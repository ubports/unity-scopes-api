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

#include <unity/scopes/internal/zmq_middleware/StateReceiverI.h>

#include <scopes/internal/zmq_middleware/capnproto/StateReceiver.capnp.h>

#include <cassert>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

/*

interface StateReceiver
{
    void push_state(std::string const& sender_id, StateReceiverObject::State state);  // oneway
};

*/

using namespace std::placeholders;

StateReceiverI::StateReceiverI(StateReceiverObject::SPtr const& sro) :
    ServantBase(sro, { { "push_state", std::bind(&StateReceiverI::push_state_, this, _1, _2, _3) } })
{
}

StateReceiverI::~StateReceiverI()
{
}

void StateReceiverI::push_state_(Current const&,
                                 capnp::AnyPointer::Reader& in_params,
                                 capnproto::Response::Builder&)
{
    auto delegate = std::dynamic_pointer_cast<StateReceiverObject>(del());
    auto req = in_params.getAs<capnproto::StateReceiver::PushStateRequest>();
    auto sender_id = req.getSenderId();
    auto s = req.getState();
    StateReceiverObject::State state;
    switch (s)
    {
        case capnproto::StateReceiver::State::SCOPE_READY:
        {
            state = StateReceiverObject::ScopeReady;
            break;
        }
        case capnproto::StateReceiver::State::SCOPE_STOPPING:
        {
            state = StateReceiverObject::ScopeStopping;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    delegate->push_state(sender_id, state);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
