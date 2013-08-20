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

#include <unity/api/scopes/internal/QueryObject.h>

#include <unity/api/scopes/internal/MWQueryCtrl.h>
#include <unity/api/scopes/internal/MWReply.h>
#include <unity/api/scopes/internal/QueryCtrlObject.h>
#include <unity/api/scopes/internal/ReplyImpl.h>
#include <unity/api/scopes/QueryBase.h>
#include <unity/Exception.h>

#include <cassert>
#include <iostream> // TODO: remove this

using namespace std;
using namespace unity::api::scopes::internal;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

QueryObject::QueryObject(shared_ptr<QueryBase> const& query_base,
                         MWReplyProxy const& reply,
                         MWQueryCtrlProxy const& ctrl) :
    query_base_(query_base),
    reply_(reply),
    ctrl_(ctrl)
{
    assert(query_base);
    assert(reply);
    assert(ctrl);
}

QueryObject::~QueryObject() noexcept
{
    assert(ctrl_);
    try
    {
        cerr << "QO: calling ctrl::destroy" << endl;
        ctrl_->destroy();
        cerr << "QO: destruction complete" << endl;
    }
    catch (...)
    {
        // TODO: log error
    }
}

void QueryObject::run(MWReplyProxy const& reply)
{
    assert(self_);

    // Create the reply proxy to pass to query_base_.
    auto reply_proxy = ReplyImpl::create(reply, self_);

    // The reply proxy now holds our reference count high, so
    // we can drop our own smart pointer.
    self_ = nullptr;

    // Synchronous call into scope implementation.
    // On return, replies for the query may still be outstanding.
    query_base_->run(reply_proxy);
}

void QueryObject::cancel(SPtr const& self)
{
    assert(self);

    // Send finished() to up-stream client to tell him the query is done.
    reply_->finished();     // Oneway, can't block

    // Forward the cancellation to any subqueries that may be running.
    cerr << "QO: forwarding cancel" << endl;
    query_base_->cancel();

    // Inform scope that its query was cancelled. This will cause run() on the query to return,
    // which triggers removal from the middleware. However, this QueryObject may still hang around
    // if there are references to it elsewhere.
    query_base_->cancelled(ReplyImpl::create(reply_, self));
}

// The point of keeping a shared_ptr to ourselves is to make sure this QueryObject cannot
// go out of scope in between being created by the Scope, and the first ReplyProxy for this
// query being created in QueryObject::run(). If the scope's run() method returns immediately,
// by the time QueryObject::run() starts executing, Scope::create_query() may already have
// returned and removed the query object from the middleware, causing this QueryObject's reference
// count to reach zero and get deallocated. So, create_query() calls set_self(), which remembers
// the the shared_ptr, increasing the refcount, and QueryObject::run() clears the shared_ptr after creating
// the ReplyProxy, which decrements the refcount again.
//
// The net-effect is that this QueryObject stays alive exactly for as long as there is at least
// one ReplyProxy for it in existence, or the scope's run() method is still executing (or both).
// Whatever happens last (run() returning or the last ReplyProxy going out of scope) deallocates
// this instance.

void QueryObject::set_self(SPtr const& self)
{
    assert(self);
    assert(!self_);
    self_ = self;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
