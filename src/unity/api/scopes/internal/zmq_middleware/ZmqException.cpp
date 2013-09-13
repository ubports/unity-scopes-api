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

#include <unity/api/scopes/internal/zmq_middleware/ZmqException.h>

#include <capnp/serialize.h>

#include <cassert>
#include <string>

using namespace std;

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
void marshal_unknown_exception(capnproto::Response::Builder& r, string const& s)
{
    capnp::MallocMessageBuilder ex;
    auto payload = ex.initRoot<capnproto::RuntimeException>();

    capnp::Text::Reader text(s.c_str());
    payload.setUnknown(text);

    r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
    r.setPayload<capnproto::RuntimeException>(ex.getRoot<capnproto::RuntimeException>());
}

void marshal_object_not_exist_exception(capnproto::Response::Builder& r,
                                        std::string const& id,
                                        std::string const& endpoint,
                                        std::string const& adapter)
{
    capnp::MallocMessageBuilder ex;
    auto payload = ex.initRoot<capnproto::RuntimeException>();

    auto one = payload.initObjectNotExist();
    auto proxy = one.initProxy();
    proxy.setIdentity(id.c_str());
    proxy.setEndpoint(endpoint.c_str());
    one.setAdapter(adapter.c_str());

    r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
    r.setPayload<capnproto::ObjectNotExistException>(ex.getRoot<capnproto::ObjectNotExistException>());
}

void marshal_operation_not_exist_exception(capnproto::Response::Builder& r,
                                           std::string const& id,
                                           std::string const& endpoint,
                                           std::string const& adapter,
                                           std::string const& op_name)
{
    capnp::MallocMessageBuilder ex;
    auto payload = ex.initRoot<capnproto::RuntimeException>();

    auto opne = payload.initOperationNotExist();
    auto proxy = opne.initProxy();
    proxy.setIdentity(id.c_str());
    proxy.setEndpoint(endpoint.c_str());
    opne.setAdapter(adapter.c_str());
    opne.setOpName(op_name.c_str());

    r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
    r.setPayload<capnproto::OperationNotExistException>(ex.getRoot<capnproto::OperationNotExistException>());
}

kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> create_unknown_response(capnp::MessageBuilder& b, string const& s)
{
    auto response = b.initRoot<capnproto::Response>();
    marshal_unknown_exception(response, s);
    return b.getSegmentsForOutput();
}

kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> create_object_not_exist_response(capnp::MessageBuilder& b,
                                                                                     string const& id,
                                                                                     string const& endpoint,
                                                                                     string const& adapter)
{
    auto response = b.initRoot<capnproto::Response>();
    marshal_object_not_exist_exception(response, id, endpoint, adapter);
    return b.getSegmentsForOutput();
}

#if 0
kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> create_operation_not_exist_response(capnp::MessageBuilder& b,
                                                                                        string const& id,
                                                                                        string const& endpoint,
                                                                                        string const& adapter,
                                                                                        string const& op_name)
{
    auto response = b.initRoot<capnproto::Response>();
    marshal_operation_not_exist_exception(response, id, endpoint, adapter, op_name);
    return b.getSegmentsForOutput();
}
#endif

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
