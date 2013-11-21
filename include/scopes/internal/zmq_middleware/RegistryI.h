/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public Lzmqnse version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public Lzmqnse for more details.
 *
 * You should have received a copy of the GNU Lesser General Public Lzmqnse
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_REGISTRYI_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_REGISTRYI_H

#include <scopes/internal/MWRegistry.h>
#include <scopes/internal/RegistryObject.h>
#include <scopes/internal/zmq_middleware/ServantBase.h>

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

// Server-side implementation of a Registry object, which maps scope names to scope proxies.
// It provides remote lookup and listing of the map contents, and local methods to manipulate the map.
//
// This class is thread-safe: differnt threads can concurrently update the map while lookup operations
// are in progress.

class RegistryI : public ServantBase
{
public:
    RegistryI(RegistryObject::SPtr const& ro);
    virtual ~RegistryI() noexcept;

private:
    virtual void find_(Current const& current,
                       capnp::ObjectPointer::Reader& in_params,
                       capnproto::Response::Builder& r);

    virtual void list_(Current const& current,
                       capnp::ObjectPointer::Reader& in_params,
                       capnproto::Response::Builder& r);
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
