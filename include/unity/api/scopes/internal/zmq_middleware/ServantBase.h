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

#ifndef UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_SERVANTBASE_H
#define UNITY_API_SCOPES_INTERNAL_ZMQMIDDLEWARE_SERVANTBASE_H

#include <capnp/dynamic.h>
#include <unity/api/scopes/internal/zmq_middleware/capnproto/Message.capnp.h>
#include <unity/util/NonCopyable.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

class AbstractObject;

namespace zmq_middleware
{

class ObjectAdapter;

struct Current
{
    std::string id;
    std::string op_name;
    ObjectAdapter* adapter;
};

class ServantBase : private util::NonCopyable
{
public:
    void safe_dispatch_(Current const& current,
                        capnp::DynamicObject::Reader& in_params,
                        capnproto::Response::Builder& r) noexcept;
    virtual ~ServantBase() noexcept;

protected:
    typedef std::function<void(Current const&,
                               capnp::DynamicObject::Reader& in_params,
                               capnproto::Response::Builder& r)> DispatchFunc;

    typedef std::unordered_map<std::string, DispatchFunc> DispatchTable;

    ServantBase(std::shared_ptr<AbstractObject> const& delegate, DispatchTable funcs);

    void dispatch_(Current const& current,
                   capnp::DynamicObject::Reader& in_params,
                   capnproto::Response::Builder& r);

    std::shared_ptr<AbstractObject> del() const noexcept;

private:
    std::shared_ptr<AbstractObject> delegate_;
    DispatchTable dispatch_table_;
};

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity

#endif
