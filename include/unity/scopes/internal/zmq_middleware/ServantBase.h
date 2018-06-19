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

#pragma once

#include <capnp/dynamic.h>
#include <scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/scopes/internal/zmq_middleware/Current.h>
#include <unity/util/NonCopyable.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace unity
{

namespace scopes
{

namespace internal
{

class AbstractObject;

namespace zmq_middleware
{

class ServantBase
{
public:
    NONCOPYABLE(ServantBase);

    void safe_dispatch_(Current const& current,
                        capnp::AnyPointer::Reader& in_params,
                        capnproto::Response::Builder& r) noexcept;
    virtual ~ServantBase();

protected:
    typedef std::function<void(Current const&,
                               capnp::AnyPointer::Reader& in_params,
                               capnproto::Response::Builder& r)> DispatchFunc;

    typedef std::unordered_map<std::string, DispatchFunc> DispatchTable;

    ServantBase(std::shared_ptr<AbstractObject> const& delegate, DispatchTable funcs);

    void dispatch_(Current const& current,
                   capnp::AnyPointer::Reader& in_params,
                   capnproto::Response::Builder& r);

    std::shared_ptr<AbstractObject> del() const noexcept;

private:
    std::shared_ptr<AbstractObject> delegate_;
    DispatchTable dispatch_table_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
