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

#include <unity/scopes/internal/QueryCtrlImpl.h>

#include <unity/scopes/internal/MiddlewareBase.h>
#include <unity/scopes/internal/MWQueryCtrl.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/ScopeExceptions.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

QueryCtrlImpl::QueryCtrlImpl(MWQueryCtrlProxy const& ctrl_proxy,
                             MWReplyProxy const& reply_proxy,
                             boost::log::sources::severity_channel_logger_mt<>& logger)
    : ObjectImpl(ctrl_proxy, logger)
    , reply_proxy_(reply_proxy)
{
    // We remember the reply proxy so, when the query is cancelled, we can
    // inform the reply object belonging to this query that the query is finished.
    assert(reply_proxy);

    ready_ = ctrl_proxy != nullptr;
    cancelled_ = false;
}

QueryCtrlImpl::~QueryCtrlImpl()
{
}

void QueryCtrlImpl::cancel()
{
    {
        lock_guard<mutex> lock(mutex_);
        if (!ready_)
        {
            // Remember that query was cancelled, so we can call
            // cancel() once set_proxy() is called.
            cancelled_ = true;
            return;
        }
    }

    try
    {
        // Forward cancellation down-stream to the query. This does not block.
        fwd()->cancel();

        // Indicate (to ourselves) that this query is complete. Calling via the MWReplyProxy ensures
        // the finished() call will be processed by a separate server-side thread,
        // so we cannot block here.
        reply_proxy_->finished(CompletionDetails(CompletionDetails::Cancelled));  // Oneway, can't block
    }
    catch (std::exception const& e)
    {
        BOOST_LOG_SEV(logger_, Logger::Error) << e.what();
    }
}

void QueryCtrlImpl::set_proxy(MWQueryCtrlProxy const& p)
{
    assert(proxy() == nullptr);
    ObjectImpl::set_proxy(p);

    bool need_cancel;

    {
        lock_guard<mutex> lock(mutex_);
        ready_ = true;
        need_cancel = cancelled_;
    } // Unlock

    if (need_cancel)
    {
        // If cancel() was called earlier, do the actual cancellation now
        // that we have the middleware proxy.
        cancel();
    }
}

MWQueryCtrlProxy QueryCtrlImpl::fwd()
{
    return dynamic_pointer_cast<MWQueryCtrl>(proxy());
}

} // namespace internal

} // namespace scopes

} // namespace unity
