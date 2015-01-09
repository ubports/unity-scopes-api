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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
*/

#include <unity/scopes/internal/ActivationQueryObject.h>

#include <unity/scopes/ActivationQueryBase.h>
#include <unity/scopes/internal/MWQueryCtrl.h>
#include <unity/scopes/internal/MWReply.h>
#include <unity/scopes/internal/RuntimeImpl.h>
#include <unity/scopes/ListenerBase.h>

#include <cassert>

using namespace std;
using namespace unity::scopes::internal;

namespace unity
{

namespace scopes
{

namespace internal
{

ActivationQueryObject::ActivationQueryObject(std::shared_ptr<ActivationQueryBase> const& act_base,
                                             MWReplyProxy const& reply,
                                             MWQueryCtrlProxy const& ctrl)
    : QueryObject(act_base, reply, ctrl)
    , act_base_(act_base)
{
}

ActivationQueryObject::~ActivationQueryObject()
{
    // parent destructor will call ctrl_->destroy()
}

void ActivationQueryObject::run(MWReplyProxy const& reply, InvokeInfo const& info) noexcept
{
    assert(self_);

    // The reply proxy now holds our reference count high, so
    // we can drop our own smart pointer and disconnect from the middleware.
    self_ = nullptr;
    disconnect();

    try
    {
        // no need for intermediate proxy (like with ReplyImpl::create),
        // since we get single return value from the public API
        // and just push it ourseleves
        auto res = act_base_->activate();
        reply->push(res.serialize());
        reply_->finished(CompletionDetails(CompletionDetails::OK));  // Oneway, can't block
    }
    catch (std::exception const& e)
    {
        BOOST_LOG(info.mw->runtime()->logger()) << "ActivationQueryObject::run(): " << e.what();
        reply_->finished(CompletionDetails(CompletionDetails::Error, e.what()));  // Oneway, can't block
    }
    catch (...)
    {
        BOOST_LOG(info.mw->runtime()->logger()) << "ActivationQueryObject::run(): unknown exception";
        reply_->finished(CompletionDetails(CompletionDetails::Error, "unknown exception"));  // Oneway, can't block
    }
}

} // namespace internal

} // namespace scopes

} // namespace unity
