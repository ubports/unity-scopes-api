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

#include <unity/scopes/internal/zmq_middleware/SigReceiverI.h>

#include <scopes/internal/zmq_middleware/capnproto/SigReceiver.capnp.h>

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

interface SigReceiver
{
    void push_signal(SigReceiverObject::Signal signal);  // oneway
};

*/

using namespace std::placeholders;

SigReceiverI::SigReceiverI(SigReceiverObject::SPtr const& sro) :
    ServantBase(sro, { { "push_signal", bind(&SigReceiverI::push_signal_, this, _1, _2, _3) } })
{
}

SigReceiverI::~SigReceiverI()
{
}

void SigReceiverI::push_signal_(Current const&,
                                capnp::AnyPointer::Reader& in_params,
                                capnproto::Response::Builder&)
{
    auto delegate = std::dynamic_pointer_cast<SigReceiverObject>(del());
    auto req = in_params.getAs<capnproto::SigReceiver::PushSignalRequest>();
    auto s = req.getSignal();
    SigReceiverObject::Signal signal;
    switch (s)
    {
        case capnproto::SigReceiver::Signal::SCOPE_STARTING:
        {
            signal = SigReceiverObject::ScopeStarting;
            break;
        }
        case capnproto::SigReceiver::Signal::SCOPE_RUNNING:
        {
            signal = SigReceiverObject::ScopeRunning;
            break;
        }
        case capnproto::SigReceiver::Signal::SCOPE_STOPPING:
        {
            signal = SigReceiverObject::ScopeStopping;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    delegate->push_signal(signal);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
