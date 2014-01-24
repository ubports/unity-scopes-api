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

#include <unity/scopes/internal/ReplyObject.h>
#include <unity/scopes/internal/ResultReplyObject.h>
#include <unity/scopes/internal/PreviewReplyObject.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/internal/AnnotationImpl.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/internal/CategorisedResultImpl.h>

#include <cassert>
#include <iostream> // TODO: remove this once logging is added

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

ReplyObject::ReplyObject(ListenerBase::SPtr const& receiver_base, RuntimeImpl const* runtime, std::string const& scope_name) :
    listener_base_(receiver_base),
    finished_(false),
    origin_scope_name_(scope_name),
    num_push_(0)
{
    assert(receiver_base);
    assert(runtime);
    reap_item_ = runtime->reply_reaper()->add([this] { this->disconnect(); });
}

ReplyObject::~ReplyObject()
{
    try
    {
        finished(ListenerBase::Finished, "");
    }
    catch (...)
    {
    }
}

void ReplyObject::push(VariantMap const& result) noexcept
{
    // We catch all exceptions so, if the application's push() method throws,
    // we can call finished(). Finished will be called exactly once, whether
    // push() or finished() throw or not.
    //
    // It is possible for a call to push() to arrive after finish() was
    // called. In particular, if the reaper times out this object while
    // a push() is in transit from the remote end, depending on scheduling
    // order, it is possible for push() to be called after finish() was
    // called. So, we enforce that finish() will be called exactly once, and that
    // push() will not be called once finished() was called.
    //
    // Calls to push() can be dispatched concurrently if the reply side is
    // configured with more than one thread. However, finished() is passed
    // to the application only only once all executing concurrent push() calls have completed.

    if (finished_.load())
    {
        return; // Ignore replies that arrive after finished().
    }

    reap_item_->refresh();

    unique_lock<mutex> lock(mutex_);
    assert(num_push_ >= 0);
    ++num_push_;
    lock.unlock(); // Forward invocations to application outside synchronization
    try
    {
        process_data(result);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << "ReplyObject::push(VariantMap): " << e.what() << endl;
        try
        {
            finished(ListenerBase::Error, e.what());
        }
        catch (...)
        {
        }
    }
    catch (...)
    {
        // TODO: log error
        cerr << "ReplyObject::push(VariantMap): unknown exception" << endl;
        try
        {
            finished(ListenerBase::Error, "unknown exception");
        }
        catch (...)
        {
        }
    }
    lock.lock();
    if (--num_push_ == 0)
    {
        idle_.notify_one();
    }
}

void ReplyObject::finished(ListenerBase::Reason r, string const& error_message) noexcept
{
    // We permit exactly one finished() call for a query. This avoids
    // a race condition where the executing down-stream query invokes
    // finished() concurrently with the QueryCtrl forwarding a cancel()
    // call to this reply's finished() method.
    if (finished_.exchange(true))
    {
        return;
    }

    // Only one thread can reach this point, any others were thrown out above.

    reap_item_->destroy();
    disconnect();               // Disconnect self from middleware, if this hasn't happened yet.

    // Wait until all currently executing calls to push() have completed.
    unique_lock<mutex> lock(mutex_);
    assert(num_push_ >= 0);
    idle_.wait(lock, [this] { return num_push_ == 0; });
    lock.unlock(); // Inform the application code that the query is complete outside synchronization.
    try
    {
        listener_base_->finished(r, error_message);
    }
    catch (std::exception const& e)
    {
        cerr << "ReplyObject::finished(): " << e.what() << endl;
        // TODO: log error
    }
    catch (...)
    {
        cerr << "ReplyObject::finished(): unknown exception" << endl;
        // TODO: log error
    }
}

std::string ReplyObject::origin_scope_name() const
{
    return origin_scope_name_;
}

} // namespace internal

} // namespace scopes

} // namespace unity