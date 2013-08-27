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

#include <unity/api/scopes/internal/ScopeImpl.h>

#include <slice/unity/api/scopes/internal/ice_middleware/Scope.h>
#include <unity/api/scopes/internal/MiddlewareBase.h>
#include <unity/api/scopes/internal/QueryCtrlImpl.h>
#include <unity/api/scopes/Scope.h>
#include <unity/Exception.h>

#include <cassert>

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
    mw_proxy_(mw_proxy),
    runtime_(runtime)
{
    assert(runtime);
}

ScopeImpl::~ScopeImpl() noexcept
{
}

// We swallow exceptions because there is nothing
// a scope can resonably do to recover.

QueryCtrlProxy ScopeImpl::create_query(string const& q, ReplyBase::SPtr const& reply) const
{
    // TODO: what if query was sent, but replies never arrive?
    //       Eventually, we have to time out and call finished(),
    //       otherwise the caller will never get rid of the reply object.
    //       Probably need to run a reaper thread, or reclaim timed-out
    //       reply objects as part of running a new query.
    //       Not that hard to do: put each reply object at the head of
    //       a doubly-linked list with a time stamp of last use.
    //       Every time the reply object is used, put it back at the head.
    //       when scanning for timed-out reply objects, traverse the list
    //       from the tail, checking time stamps and kill any objects that
    //       are to old (evictor pattern).
    QueryCtrlProxy ctrl;
    try
    {
        // Create a middleware server-side object that can receive incoming
        // push() and finished() messages over the network.
        ReplyObject::SPtr ro(new ReplyObject(reply, runtime_));
        MWReplyProxy rp = mw_proxy_->mw_base()->add_reply_object(ro); // ResourcePtr to remove it if exception?

        // Forward the the create_query() method across the bus. This is a
        // synchronous twoway interaction with the scope, so it can return
        // the QueryCtrlProxy. Because the Scope implementation has a separate
        // thread for create_query() calls, this is guaranteed not to block for
        // any length of time. (No application code other than the QueryBase constructor
        // is called by create_query() on the server side.)
        ctrl = mw_proxy_->create_query(q, rp);
        assert(ctrl);
    }
    catch (unity::Exception const& e)
    {
        // TODO: log error
        try
        {
            // TODO: if things go wrong, we need to make sure that the reply object
            // is disconnected from the middleware, so it gets deallocated.
            reply->finished();
            throw;
        }
        catch (...)
        {
        }
        throw;
    }
    return ctrl;
}

ScopeProxy ScopeImpl::create(MWScopeProxy const& mw_proxy, RuntimeImpl* runtime)
{
    return ScopeProxy(new Scope(new ScopeImpl(mw_proxy, runtime)));
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
