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

#include <scopes/internal/QueryObject.h>

#include <scopes/internal/MWQueryCtrl.h>
#include <scopes/internal/MWReply.h>
#include <scopes/internal/QueryCtrlObject.h>
#include <scopes/internal/ReplyImpl.h>
#include <scopes/QueryBase.h>
#include <scopes/Reply.h>
#include <unity/Exception.h>

#include <iostream>
#include <cassert>

using namespace std;
using namespace unity::scopes::internal;

namespace unity
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
    ctrl_(ctrl),
    pushable_(true)
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
        ctrl_->destroy(); // Oneway, won't block
    }
    catch (...)
    {
        // TODO: log error
    }
}

void QueryObject::run(MWReplyProxy const& reply) noexcept
{
    assert(self_);

    // Create the reply proxy to pass to query_base_ and keep a weak_ptr, which we will need
    // if cancel() is called later.
    auto reply_proxy = ReplyImpl::create(reply, self_);
    assert(reply_proxy);
    reply_proxy_ = reply_proxy;

    // The reply proxy now holds our reference count high, so
    // we can drop our own smart pointer and disconnect from the middleware.
    self_ = nullptr;
    disconnect();

    // Synchronous call into scope implementation.
    // On return, replies for the query may still be outstanding.
    try
    {
        query_base_->run(reply_proxy);
    }
    catch (std::exception const& e)
    {
        pushable_ = false;
        // TODO: log error
        reply_->finished(ReceiverBase::Error, e.what());     // Oneway, can't block
        cerr << "ScopeBase::run(): " << e.what() << endl;
    }
    catch (...)
    {
        pushable_ = false;
        // TODO: log error
        reply_->finished(ReceiverBase::Error, "unknown exception");     // Oneway, can't block
        cerr << "ScopeBase::run(): unknown exception" << endl;
    }
}

void QueryObject::cancel()
{
    pushable_ = false;
    auto rp = reply_proxy_.lock();
    if (rp)
    {
        // Send finished() to up-stream client to tell him the query is done.
        // We send via the MWReplyProxy here because that allows passing
        // a ReceiverBase::Reason (whereas the public ReplyProxy does not).
        reply_->finished(ReceiverBase::Cancelled, "");     // Oneway, can't block
    }

    // Forward the cancellation to the query base (which in turn will forward it to any subqueries).
    // The query base also calls the cancelled() callback to inform the application code.
    query_base_->cancel();
}

bool QueryObject::pushable() const noexcept
{
    return pushable_;
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

} // namespace unity
