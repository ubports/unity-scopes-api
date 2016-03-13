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
 * Authored by: Michal Hruby <michal.hruby@canonical.com>
*/

#include <unity/scopes/internal/PreviewQueryObject.h>

#include <unity/scopes/internal/MWQueryCtrl.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/PreviewReplyImpl.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>

#include <cassert>

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

PreviewQueryObject::PreviewQueryObject(std::shared_ptr<PreviewQueryBase> const& preview_base,
                                       MWReplyProxy const& reply,
                                       MWQueryCtrlProxy const& ctrl)
    : QueryObject(preview_base, reply, ctrl)
    , preview_base_(preview_base)
{
    assert(preview_base);
}

PreviewQueryObject::~PreviewQueryObject()
{
    // parent destructor will call ctrl_->destroy()
}

void PreviewQueryObject::run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept
{
    unique_lock<mutex> lock(mutex_);

    // See comment in QueryObject::run()
    if (!pushable_)
    {
        self_ = nullptr;
        disconnect();
        return;
    }

    assert(self_);

    auto reply_proxy = make_shared<PreviewReplyImpl>(reply, self_);
    assert(reply_proxy);
    reply_proxy_ = reply_proxy;

    // The reply proxy now holds our reference count high, so
    // we can drop our own smart pointer and disconnect from the middleware.
    self_ = nullptr;
    disconnect();

    try
    {
        lock.unlock();

        // Synchronous call into scope implementation.
        // On return, replies for the preview may still be outstanding.
        auto preview_query = dynamic_pointer_cast<PreviewQueryBase>(query_base_);
        assert(preview_query);
        preview_query->run(reply_proxy);
    }
    catch (std::exception const& e)
    {
        {
            lock_guard<mutex> lock(mutex_);
            pushable_ = false;
        }
        info.mw->runtime()->logger()() << "PreviewQueryBase::run(): " << e.what();
        reply_->finished(CompletionDetails(CompletionDetails::Error, string("PreviewQueryBase::run(): ") + e.what()));
    }
    catch (...)
    {
        {
            lock_guard<mutex> lock(mutex_);
            pushable_ = false;
        }
        info.mw->runtime()->logger()() << "PreviewQueryBase::run(): unknown exception";
        reply_->finished(CompletionDetails(CompletionDetails::Error, "PreviewQueryBase::run(): unknown exception"));
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
