/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQEXCEPTION_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_ZMQEXCEPTION_H

#include <capnp/message.h>
#include <scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/util/NonCopyable.h>

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

void marshal_unknown_exception(capnproto::Response::Builder& r, std::string const& s);
void marshal_object_not_exist_exception(capnproto::Response::Builder& r,
                                        std::string const& id,
                                        std::string const& endpoint,
                                        std::string const& adapter);
void marshal_operation_not_exist_exception(capnproto::Response::Builder& r,
                                           std::string const& id,
                                           std::string const& endpoint,
                                           std::string const& adapter,
                                           std::string const& op_name);

kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> create_unknown_response(capnp::MessageBuilder& b,
                                                                            std::string const& s);
kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> create_object_not_exist_response(capnp::MessageBuilder& b,
                                                                                     std::string const& id,
                                                                                     std::string const& endpoint,
                                                                                     std::string const& adapter);

void throw_if_runtime_exception(capnproto::Response::Reader const& reader);

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
