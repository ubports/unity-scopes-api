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

#include <unity/scopes/internal/zmq_middleware/ZmqStateReceiver.h>

#include <scopes/internal/zmq_middleware/capnproto/StateReceiver.capnp.h>

#include <capnp/message.h>

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

ZmqStateReceiver::ZmqStateReceiver(ZmqMiddleware* mw_base,
                                   std::string const& endpoint,
                                   std::string const& identity,
                                   std::string const& category) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, category, RequestMode::Oneway),
    MWStateReceiver(mw_base)
{
}

ZmqStateReceiver::~ZmqStateReceiver()
{
}

void ZmqStateReceiver::push_state(std::string const& sender_id, StateReceiverObject::State const& state)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "push_state");
    auto in_params = request.initInParams().getAs<capnproto::StateReceiver::PushStateRequest>();
    capnproto::StateReceiver::State s;
    switch (state)
    {
        case StateReceiverObject::ScopeReady:
        {
            s = capnproto::StateReceiver::State::SCOPE_READY;
            break;
        }
        case StateReceiverObject::ScopeStopping:
        {
            s = capnproto::StateReceiver::State::SCOPE_STOPPING;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    in_params.setState(s);
    in_params.setSenderId(sender_id);

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_oneway_(request_builder); });
    future.wait();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
