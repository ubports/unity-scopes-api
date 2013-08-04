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

#include <unity/api/scopes/internal/ScopeProxyImpl.h>

#include <unity/api/scopes/internal/MiddlewareBase.h>
#include <unity/Exception.h>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

ScopeProxyImpl::ScopeProxyImpl(MWScopeProxy::SPtr const& mw_proxy) :
    mw_proxy_(mw_proxy)
{
}

ScopeProxyImpl::~ScopeProxyImpl() noexcept
{
}

// We swallow middleware exceptions because there is nothing
// a scope can resonably do to recover.

void ScopeProxyImpl::query(string const& q, ReplyBase::SPtr const& reply) const
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
    try
    {
        ReplyObject::SPtr ro(new ReplyObject(reply));
        MWReplyProxy::SPtr rp = mw_proxy_->mw_base()->add_reply_object(ro);
        mw_proxy_->query(q, rp);
    }
    catch (unity::Exception const& e)
    {
        try
        {
            reply->finished();
        }
        catch (...)
        {
        }
        // TODO: log error
    }
}

ScopeProxy::SPtr ScopeProxyImpl::create(MWScopeProxy::SPtr const& mw_proxy)
{
    return ScopeProxy::SPtr(new ScopeProxy(new ScopeProxyImpl(mw_proxy)));
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
