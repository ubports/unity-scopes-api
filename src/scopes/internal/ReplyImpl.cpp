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

#include <unity/scopes/internal/ReplyImpl.h>

#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/QueryObjectBase.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/Reply.h>
#include <unity/UnityExceptions.h>

#include <cassert>
#include <iostream> // TODO: remove this once logging is added

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

ReplyImpl::ReplyImpl(MWReplyProxy const& mw_proxy, std::shared_ptr<QueryObjectBase> const& qo) :
    ObjectImpl(mw_proxy),
    qo_(qo),
    finished_(false),
    pushes_busy_(0)
{
    assert(mw_proxy);
}

ReplyImpl::~ReplyImpl()
{
    try
    {
        finished();
    }
    catch (...)
    {
        // TODO: log error
    }
}

bool ReplyImpl::push(VariantMap const& variant_map)
{
    // Increment pushes_busy_
    {
        std::lock_guard<std::mutex> push_lock(push_mutex_);
        ++pushes_busy_;
    }

    bool result = true;
    auto qo = dynamic_pointer_cast<QueryObjectBase>(qo_);
    assert(qo);
    if (finished_ || !qo->pushable({ fwd()->identity(), fwd()->mw_base() }))
    {
        result = false; // Query was finished, cancelled or had an error.
    }
    else
    {
        try
        {
            fwd()->push(variant_map);
        }
        catch (std::exception const&)
        {
            error(current_exception());
            result = false;
        }
    }

    // Decrement pushes_busy_ and signal that a push completed
    {
        std::lock_guard<std::mutex> push_lock(push_mutex_);
        --pushes_busy_;
        push_cond_.notify_one();
    }

    return result;
}

void ReplyImpl::finished()
{
    finished(ListenerBase::Finished);
}

void ReplyImpl::finished(ListenerBase::Reason reason)
{
    // If any pushes are currently busy, give them a second to complete
    {
        std::unique_lock<std::mutex> push_lock(push_mutex_);
        push_cond_.wait_for(push_lock, std::chrono::seconds(1), [this] { return pushes_busy_ == 0; });
    }

    if (!finished_.exchange(true))
    {
        try
        {
            fwd()->finished(reason, "");
        }
        catch (std::exception const& e)
        {
            // TODO: log error
            cerr << e.what() << endl;
        }
    }
}

void ReplyImpl::error(exception_ptr ex)
{
    if (finished_.exchange(true))
    {
        // Only the first thread to encounter an error
        // reports the error to the client.
        return;
    }

    string error_message;
    try
    {
        rethrow_exception(ex);
    }
    catch (std::exception const& e)
    {
        error_message = e.what();
    }
    catch (...)
    {
        error_message = "unknown exception";
    }
    // TODO: log error
    cerr << error_message << endl;

    try
    {
        fwd()->finished(ListenerBase::Error, error_message);
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        cerr << e.what() << endl;
    }
}

MWReplyProxy ReplyImpl::fwd() const
{
    return dynamic_pointer_cast<MWReply>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace unity
