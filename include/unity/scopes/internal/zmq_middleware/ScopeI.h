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

#ifndef UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_SCOPEI_H
#define UNITY_SCOPES_INTERNAL_ZMQMIDDLEWARE_SCOPEI_H

#include <unity/scopes/internal/MWScope.h>
#include <unity/scopes/internal/ScopeObjectBase.h>
#include <unity/scopes/internal/zmq_middleware/ServantBase.h>

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

class ScopeI : public ServantBase
{
public:
    ScopeI(ScopeObjectBase::SPtr const& so);
    virtual ~ScopeI();

private:
    virtual void search_(Current const& current,
                               capnp::AnyPointer::Reader& in_params,
                               capnproto::Response::Builder& r);
    virtual void activate_(Current const& current,
                           capnp::AnyPointer::Reader& in_params,
                           capnproto::Response::Builder& r);
    virtual void perform_action_(Current const& current,
                           capnp::AnyPointer::Reader& in_params,
                           capnproto::Response::Builder& r);
    virtual void preview_(Current const& current,
                          capnp::AnyPointer::Reader& in_params,
                          capnproto::Response::Builder& r);
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
