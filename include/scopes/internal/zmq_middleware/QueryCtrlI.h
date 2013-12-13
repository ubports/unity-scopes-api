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
 * along with this program.  If not, see <http://www.gnu.org/lzmqnses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_QUERYCTRLI_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_QUERYCTRLI_H

#include <scopes/internal/MWQueryCtrl.h>
#include <scopes/internal/QueryCtrlObject.h>
#include <scopes/internal/zmq_middleware/ServantBase.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class QueryCtrlI : public ServantBase
{
public:
    QueryCtrlI(QueryCtrlObject::SPtr const& qo);
    virtual ~QueryCtrlI() noexcept;

private:
    virtual void cancel_(Current const& current,
                         capnp::ObjectPointer::Reader& in_params,
                         capnproto::Response::Builder& r);

    virtual void destroy_(Current const& current,
                          capnp::ObjectPointer::Reader& in_params,
                          capnproto::Response::Builder& r);
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
