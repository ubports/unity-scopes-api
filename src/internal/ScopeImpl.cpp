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

#include <scopes/internal/ScopeImpl.h>

#include <scopes/internal/MiddlewareBase.h>
#include <scopes/internal/MWScope.h>
#include <scopes/internal/QueryCtrlImpl.h>
//#include <scopes/internal/PreviewCtrlImpl.h>
#include <scopes/Scope.h>
#include <unity/Exception.h>

#include <cassert>
#include <iostream> // TODO: remove this once logging is added

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ScopeImpl::ScopeImpl(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime) :
    ObjectProxyImpl(mw_proxy),
    runtime_(runtime)
{
    assert(runtime);
}

ScopeImpl::~ScopeImpl() noexcept
{
}

QueryCtrlProxy ScopeImpl::create_query(string const& q, VariantMap const& hints, SearchListener::SPtr const& reply) const
{
    QueryCtrlProxy ctrl;
    try
    {
        // Create a middleware server-side object that can receive incoming
        // push() and finished() messages over the network.
        ReplyObject::SPtr ro(make_shared<ResultReplyObject>(reply, runtime_));
        MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

        // Forward the the create_query() method across the bus. This is a
        // synchronous twoway interaction with the scope, so it can return
        // the QueryCtrlProxy. Because the Scope implementation has a separate
        // thread for create_query() calls, this is guaranteed not to block for
        // any length of time. (No application code other than the QueryBase constructor
        // is called by create_query() on the server side.)
        ctrl = fwd()->create_query(q, hints, rp);
        assert(ctrl);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "create_query(): " << e.what() << endl;
        try
        {
            // TODO: if things go wrong, we need to make sure that the reply object
            // is disconnected from the middleware, so it gets deallocated.
            reply->finished(ListenerBase::Error, e.what());
            throw;
        }
        catch (...)
        {
            cerr << "create_query(): unknown exception" << endl;
        }
        throw;
    }
    return ctrl;
}

QueryCtrlProxy ScopeImpl::preview(Result const& result, VariantMap const& hints, PreviewListener::SPtr const& reply) const
{
    QueryCtrlProxy ctrl;
    try
    {
        // Create a middleware server-side object that can receive incoming
        // push() and finished() messages over the network.
        PreviewReplyObject::SPtr ro(make_shared<PreviewReplyObject>(reply, runtime_));
        MWReplyProxy rp = fwd()->mw_base()->add_reply_object(ro);

        // Forward the the create_query() method across the bus. This is a
        // synchronous twoway interaction with the scope, so it can return
        // the QueryCtrlProxy. Because the Scope implementation has a separate
        // thread for create_query() calls, this is guaranteed not to block for
        // any length of time. (No application code other than the QueryBase constructor
        // is called by create_query() on the server side.)
        ctrl = fwd()->preview(result, hints, rp);
        assert(ctrl);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "preview(): " << e.what() << endl;
        try
        {
            // TODO: if things go wrong, we need to make sure that the reply object
            // is disconnected from the middleware, so it gets deallocated.
            reply->finished(ListenerBase::Error, e.what());
            throw;
        }
        catch (...)
        {
            cerr << "preview(): unknown exception" << endl;
        }
        throw;
    }
    return ctrl;
}

ScopeProxy ScopeImpl::create(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime)
{
    return ScopeProxy(new Scope(new ScopeImpl(mw_proxy, runtime)));
}

MWScopeProxy ScopeImpl::fwd() const
{
    return dynamic_pointer_cast<MWScope>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
