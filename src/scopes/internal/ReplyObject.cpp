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

ReplyObject::ReplyObject(ListenerBase::SPtr const& receiver_base, RuntimeImpl const* runtime,
                         std::string const& scope_proxy, bool dont_reap) :
    listener_base_(receiver_base),
    finished_(false),
    origin_proxy_(scope_proxy),
    num_push_(0),
    info_occurred_(false)
{
    assert(receiver_base);
    assert(runtime);

    if (dont_reap == false)
    {
        unique_lock<mutex> lock(mutex_);  // Make sure that when lambda fires, it sees current values.
        reap_item_ = runtime->reply_reaper()->add([this] {
            string msg = "No activity on ReplyObject for scope " + this->origin_proxy_ + ": ReplyObject destroyed";
            this->finished(CompletionDetails(CompletionDetails::Error, msg));
        });
    }
}

ReplyObject::~ReplyObject()
{
    try
    {
        finished(CompletionDetails(CompletionDetails::OK));
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
    // to the application only once all executing concurrent push() calls have completed.

    if (finished_.load())
    {
        return; // Ignore replies that arrive after finished().
    }

    if (reap_item_)
    {
        reap_item_->refresh();
    }

    {
        unique_lock<mutex> lock(mutex_);
        assert(num_push_ >= 0);
        ++num_push_;
    }  // Forward invocations to application outside synchronization

    bool stop = false;
    string error;
    try
    {
        stop = process_data(result);  // Returns true if cardinality limit was reached
    }
    catch (std::exception const& e)
    {
        error = string("ReplyObject::push(VariantMap): ") + e.what();
    }
    catch (...)
    {
        error = "ReplyObject::push(VariantMap): unknown exception";
    }

    // Decrement number of pushes before potentially calling finished(),
    // because finished() waits for concurrent push() calls to complete.
    {
        unique_lock<mutex> lock(mutex_);
        if (--num_push_ == 0)
        {
            idle_.notify_one();
        }
    }

    // Safe to call finished now if something went wrong or cardinality was exceeded.
    if (!error.empty())
    {
        // TODO: log error
        finished(CompletionDetails(CompletionDetails::Error, error));
    }
    else if (stop)
    {
        // TODO: log error
        finished(CompletionDetails(CompletionDetails::OK));
    }
}

void ReplyObject::finished(CompletionDetails const& details) noexcept
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

    ReapItem::SPtr ri;
    {
        unique_lock<mutex> lock(mutex_);  // If finished() is called by reaper, the
        ri = reap_item_;                  // callback needs to see the current value of reap_item_.
    }
    if (ri)
    {
        ri->cancel();
    }

    // Wait until all currently executing calls to push() have completed.
    unique_lock<mutex> lock(mutex_);
    assert(num_push_ >= 0);
    idle_.wait(lock, [this] { return num_push_ == 0; });
    lock.unlock(); // Inform the application code that the query is complete outside synchronization.
    try
    {
        CompletionDetails details_with_info(details);
        for (auto const& info : info_list_)
        {
            details_with_info.add_info(info);
        }
        listener_base_->finished(details_with_info);
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

    // Disconnect self from middleware, if this hasn't happened yet.
    // We do this last because disconnect() can trigger
    // the destructor of this instance (if finished() is called
    // by the reaper, for example).
    disconnect();
}

void ReplyObject::info(OperationInfo const& op_info) noexcept
{
    if (finished_.load())
    {
        return; // Ignore info messages that arrive after finished().
    }

    if (reap_item_)
    {
        reap_item_->refresh();
    }
    info_occurred_.exchange(true);

    try
    {
        info_list_.push_back(op_info);
        listener_base_->info(op_info);
    }
    catch (std::exception const& e)
    {
        cerr << "ReplyObject::info(): " << e.what() << endl;
        // TODO: log error
    }
    catch (...)
    {
        cerr << "ReplyObject::info(): unknown exception" << endl;
        // TODO: log error
    }
}

std::string ReplyObject::origin_proxy() const
{
    return origin_proxy_;
}

} // namespace internal

} // namespace scopes

} // namespace unity
