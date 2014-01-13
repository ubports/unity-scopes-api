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

#include <scopes/internal/ActivationQueryObject.h>
#include <scopes/ListenerBase.h>
#include <scopes/ActivationBase.h>
#include <scopes/internal/MWReply.h>
#include <scopes/internal/MWQueryCtrl.h>
#include <iostream>
#include <cassert>

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

ActivationQueryObject::ActivationQueryObject(std::shared_ptr<ActivationBase> const& act_base, MWReplyProxy const& reply, MWQueryCtrlProxy const& ctrl)
    : QueryObjectBase(),
    act_base_(act_base),
    reply_(reply),
    ctrl_(ctrl)
{
}

ActivationQueryObject::~ActivationQueryObject() noexcept
{
    try
    {
        ctrl_->destroy(); // Oneway, won't block
    }
    catch (...)
    {
        // TODO: log error
    }
}

void ActivationQueryObject::cancel()
{
    // Send finished() to up-stream client to tell him the query is done.
    // We send via the MWReplyProxy here because that allows passing
    // a ListenerBase::Reason (whereas the public ReplyProxy does not).
    reply_->finished(ListenerBase::Cancelled, "");     // Oneway, can't block
}

void ActivationQueryObject::run(MWReplyProxy const& reply) noexcept
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
        reply_->finished(ListenerBase::Finished, "");
    }
    catch (std::exception const& e)
    {
        // TODO: log error
        reply_->finished(ListenerBase::Error, e.what());     // Oneway, can't block
        cerr << "ActivationQueryObject::run(): " << e.what() << endl;
    }
    catch (...)
    {
        // TODO: log error
        reply_->finished(ListenerBase::Error, "unknown exception");     // Oneway, can't block
        cerr << "ActivationQueryObject::run(): unknown exception" << endl;
    }
}

void ActivationQueryObject::set_self(SPtr const& self)
{
    assert(self);
    assert(!self_);
    self_ = self;
}

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
