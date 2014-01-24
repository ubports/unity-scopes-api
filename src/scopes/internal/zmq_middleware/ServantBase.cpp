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

#include <unity/scopes/internal/zmq_middleware/ServantBase.h>

#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/Exception.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

using namespace std::placeholders;

ServantBase::ServantBase(shared_ptr<AbstractObject> const& delegate, DispatchTable funcs) :
    delegate_(delegate),
    dispatch_table_(move(funcs))
{
    assert(delegate);
}

ServantBase::~ServantBase()
{
}

void ServantBase::dispatch_(Current const& current,
                            capnp::AnyPointer::Reader& in_params,
                            capnproto::Response::Builder& r)
{
    auto it = dispatch_table_.find(current.op_name);
    if (it == dispatch_table_.end())
    {
        if (current.op_name == "ping")
        {
            r.setStatus(capnproto::ResponseStatus::SUCCESS);
            return;
        }
        marshal_operation_not_exist_exception(r, current);
        return;
    }
    auto dispatch_func = it->second;
    dispatch_func(current, in_params, r);
}

// Simple forwarding function around the dispatch_ method in a servant class.
// It ensures that, if the application throws an exception, we marshal something
// sensible back to the client.

void ServantBase::safe_dispatch_(Current const& current,
                                 capnp::AnyPointer::Reader& in_params,
                                 capnproto::Response::Builder& r) noexcept
{
    string error;
    try
    {
        dispatch_(current, in_params, r);
        return;
    }
    catch (std::exception const& e)
    {
        error = e.what();
    }
    catch (...)
    {
        error = "unknown exception";
    }
    marshal_unknown_exception(r, error);
    r.setStatus(capnproto::ResponseStatus::RUNTIME_EXCEPTION);
}

shared_ptr<AbstractObject> ServantBase::del() const noexcept
{
    return delegate_;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
