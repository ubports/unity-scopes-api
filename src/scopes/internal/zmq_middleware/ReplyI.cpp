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

#include <unity/scopes/internal/zmq_middleware/ReplyI.h>

#include <scopes/internal/zmq_middleware/capnproto/Reply.capnp.h>
#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>

using namespace std;

namespace unity
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

/*

interface Reply
{
    void push(string result);
    void finished();
};

*/

using namespace std::placeholders;

ReplyI::ReplyI(ReplyObjectBase::SPtr const& ro) :
    ServantBase(ro, { { "push", bind(&ReplyI::push_, this, _1, _2, _3) },
                      { "finished", bind(&ReplyI::finished_, this, _1, _2, _3) } }),
    pushes_busy_(0)
{
}

ReplyI::~ReplyI()
{
}

void ReplyI::push_(Current const&,
                   capnp::AnyPointer::Reader& in_params,
                   capnproto::Response::Builder&)
{
    // Increment pushes_busy_
    {
        std::lock_guard<std::mutex> push_lock(push_mutex_);
        ++pushes_busy_;
    }

    auto req = in_params.getAs<capnproto::Reply::PushRequest>();
    auto result = req.getResult();
    auto delegate = dynamic_pointer_cast<ReplyObjectBase>(del());
    delegate->push(to_variant_map(result));

    // Decrement pushes_busy_ and signal that a push completed
    {
        std::lock_guard<std::mutex> push_lock(push_mutex_);
        --pushes_busy_;
        push_cond_.notify_one();
    }
}

void ReplyI::finished_(Current const&,
                       capnp::AnyPointer::Reader& in_params,
                       capnproto::Response::Builder&)
{
    // If any pushes are currently busy, give them a second to complete
    {
        std::unique_lock<std::mutex> push_lock(push_mutex_);
        push_cond_.wait_for(push_lock, std::chrono::seconds(1), [this] { return pushes_busy_ == 0; });
    }

    auto delegate = dynamic_pointer_cast<ReplyObjectBase>(del());
    auto req = in_params.getAs<capnproto::Reply::FinishedRequest>();
    auto r = req.getReason();
    ListenerBase::Reason reason;
    string err;
    switch (r)
    {
        case capnproto::Reply::FinishedReason::FINISHED:
        {
            reason = ListenerBase::Finished;
            break;
        }
        case capnproto::Reply::FinishedReason::CANCELLED:
        {
            reason = ListenerBase::Cancelled;
            break;
        }
        case capnproto::Reply::FinishedReason::ERROR:
        {
            reason = ListenerBase::Error;
            err = req.getError();
            break;
        }
        default:
        {
            assert(false);                // LCOV_EXCL_LINE
            reason = ListenerBase::Error; // LCOV_EXCL_LINE
        }
    }
    delegate->finished(reason, err);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
