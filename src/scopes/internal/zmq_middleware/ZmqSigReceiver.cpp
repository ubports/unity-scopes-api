/*
 * Copyright (C) 2013 Canonical Ltd
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

#include <unity/scopes/internal/zmq_middleware/ZmqSigReceiver.h>

#include <scopes/internal/zmq_middleware/capnproto/SigReceiver.capnp.h>

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

///! TODO

*/

ZmqSigReceiver::ZmqSigReceiver(ZmqMiddleware* mw_base,
                               std::string const& endpoint,
                               std::string const& identity,
                               std::string const& category,
                               int64_t timeout) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, category, RequestMode::Twoway, timeout),
    MWSigReceiver(mw_base)
{
}

ZmqSigReceiver::~ZmqSigReceiver()
{
}

void ZmqSigReceiver::push_signal(SigReceiverObject::Signal const& signal)
{
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
