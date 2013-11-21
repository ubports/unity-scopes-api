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

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQRECEIVER_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQRECEIVER_H

#include <unity/util/NonCopyable.h>
#include <capnp/common.h>
#include <zmqpp/socket.hpp>

#include <memory>
#include <vector>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

// Simple message receiver. Converts a message received from zmq (either as a single message or in parts)
// to a Cap'n Proto segment list, taking care of any alignment issues. The receiver instance must stay
// in scope until unmarshaling is complete.

class ZmqReceiver final : private util::NonCopyable
{
public:
    ZmqReceiver(zmqpp::socket& s);

    kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> receive();

private:
    zmqpp::socket& s_;
    std::vector<std::string> parts_;
    std::vector<std::unique_ptr<capnp::word[]>> copied_parts_;
    std::vector<kj::ArrayPtr<capnp::word const>> segments_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
