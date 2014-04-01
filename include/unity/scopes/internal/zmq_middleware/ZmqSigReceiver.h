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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQSIGRECEIVER_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQSIGRECEIVER_H

#include <unity/scopes/internal/zmq_middleware/ZmqObjectProxy.h>
#include <unity/scopes/internal/zmq_middleware/ZmqSigReceiverProxyFwd.h>
#include <unity/scopes/internal/MWSigReceiver.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ZmqSigReceiver : public virtual ZmqObjectProxy, public virtual MWSigReceiver
{
public:
    ZmqSigReceiver(ZmqMiddleware* mw_base,
                   std::string const& endpoint,
                   std::string const& identity,
                   std::string const& category);
    virtual ~ZmqSigReceiver();

    void push_signal(std::string const& sender_id, SigReceiverObject::SignalType const& signal) override;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif // UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQSIGRECEIVER_H
