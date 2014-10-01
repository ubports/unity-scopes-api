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
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQSENDER_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQSENDER_H

#include <unity/util/NonCopyable.h>
#include <capnp/common.h>
#include <zmqpp/socket.hpp>

#include <memory>
#include <vector>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

// Simple message sender. Sends a Cap'n Proto segment list sending each segment as a zmq message part.

class ZmqSender final
{
public:
    NONCOPYABLE(ZmqSender);

    ZmqSender(zmqpp::socket& s);

    enum WaitFlag { Wait, DontWait };

    bool send(kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> segments, WaitFlag flag = Wait);

private:
    zmqpp::socket& s_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
