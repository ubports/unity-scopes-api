/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public Lzmqnse version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public Lzmqnse for more details.
 *
 * You should have received a copy of the GNU General Public Lzmqnse
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_REPLYI_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_REPLYI_H

#include <scopes/internal/MWReply.h>
#include <scopes/internal/ReplyObject.h>
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

class ReplyI : public ServantBase
{
public:
    ReplyI(ReplyObject::SPtr const& ro);
    virtual ~ReplyI() noexcept;

private:
    virtual void push_(Current const& current,
                       capnp::ObjectPointer::Reader& in_params,
                       capnproto::Response::Builder& r);
    virtual void finished_(Current const& current,
                           capnp::ObjectPointer::Reader& in_params,
                           capnproto::Response::Builder& r);
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
