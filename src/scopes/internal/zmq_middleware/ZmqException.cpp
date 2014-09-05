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

#include <unity/scopes/internal/zmq_middleware/ZmqException.h>

#include <unity/scopes/internal/zmq_middleware/Current.h>
#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/ScopeExceptions.h>

#include <capnp/serialize.h>

#include <cassert>
#include <string>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

void marshal_unknown_exception(capnproto::Response::Builder& r, string const& s)
{
    auto payload = r.initPayload().initAs<capnproto::RuntimeException>();

    payload.setUnknown(s);
    r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
}

void marshal_object_not_exist_exception(capnproto::Response::Builder& r, Current const& c)
{
    auto payload = r.initPayload().initAs<capnproto::RuntimeException>();

    auto one = payload.initObjectNotExist();
    auto proxy = one.initProxy();
    proxy.setIdentity(c.id.c_str());
    proxy.setEndpoint(c.adapter->endpoint().c_str());
    proxy.setCategory(c.category.c_str());
    one.setAdapter(c.adapter->name().c_str());

    r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
}

void marshal_operation_not_exist_exception(capnproto::Response::Builder& r, Current const& c)
{
    auto payload = r.initPayload().initAs<capnproto::RuntimeException>();

    auto opne = payload.initOperationNotExist();
    auto proxy = opne.initProxy();
    proxy.setIdentity(c.id.c_str());
    proxy.setEndpoint(c.adapter->endpoint().c_str());
    proxy.setCategory(c.category.c_str());
    opne.setAdapter(c.adapter->name().c_str());
    opne.setOpName(c.op_name.c_str());

    r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
}

kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> create_unknown_response(capnp::MessageBuilder& b, string const& s)
{
    auto response = b.initRoot<capnproto::Response>();
    marshal_unknown_exception(response, s);
    return b.getSegmentsForOutput();
}

kj::ArrayPtr<kj::ArrayPtr<capnp::word const> const> create_object_not_exist_response(capnp::MessageBuilder& b,
                                                                                     Current const& c)
{
    auto response = b.initRoot<capnproto::Response>();
    marshal_object_not_exist_exception(response, c);
    return b.getSegmentsForOutput();
}

void throw_if_runtime_exception(capnproto::Response::Reader const& response)
{
    if (response.getStatus() != capnproto::ResponseStatus::RUNTIME_EXCEPTION)
    {
        return;
    }
    auto payload = response.getPayload().getAs<capnproto::RuntimeException>();
    switch (payload.which())
    {
        case capnproto::RuntimeException::OPERATION_NOT_EXIST:
        {
            auto opne = payload.getOperationNotExist();
            auto proxy = opne.getProxy();
            string msg = string("Operation \"") + opne.getOpName().cStr() + "\" does not exist "
                         "(adapter = " + opne.getAdapter().cStr() + ", endpoint = " +
                         proxy.getEndpoint().cStr() + ", identity = " + proxy.getIdentity().cStr() + ")";
            throw MiddlewareException(msg);
        }
        case capnproto::RuntimeException::OBJECT_NOT_EXIST:
        {
            auto one = payload.getObjectNotExist();
            auto proxy = one.getProxy();
            string msg = string("Object does not exist (adapter = ") + one.getAdapter().cStr() + ", endpoint = " +
                         proxy.getEndpoint().cStr() + ", identity = " + proxy.getIdentity().cStr() + ")";
            throw ObjectNotExistException(msg, proxy.getIdentity().cStr());
        }
        case capnproto::RuntimeException::UNKNOWN:
        {
            throw MiddlewareException(payload.getUnknown().cStr());
        }
        default:
            break;
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
