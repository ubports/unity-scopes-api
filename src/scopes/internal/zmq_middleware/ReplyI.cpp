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
                      { "finished", bind(&ReplyI::finished_, this, _1, _2, _3) },
                      { "info", bind(&ReplyI::info_, this, _1, _2, _3) } })
{
}

ReplyI::~ReplyI()
{
}

void ReplyI::push_(Current const&,
                   capnp::AnyPointer::Reader& in_params,
                   capnproto::Response::Builder&)
{
    auto req = in_params.getAs<capnproto::Reply::PushRequest>();
    auto result = req.getResult();
    auto delegate = dynamic_pointer_cast<ReplyObjectBase>(del());
    delegate->push(to_variant_map(result));
}

void ReplyI::finished_(Current const&,
                       capnp::AnyPointer::Reader& in_params,
                       capnproto::Response::Builder&)
{///!
//    auto delegate = dynamic_pointer_cast<ReplyObjectBase>(del());
//    auto req = in_params.getAs<capnproto::Reply::FinishedRequest>();
//    auto r = req.getReason();
//    ListenerBase::Reason reason;
//    string err;
//    switch (r)
//    {
//        case capnproto::Reply::FinishedReason::FINISHED:
//        {
//            reason = ListenerBase::Finished;
//            break;
//        }
//        case capnproto::Reply::FinishedReason::CANCELLED:
//        {
//            reason = ListenerBase::Cancelled;
//            break;
//        }
//        case capnproto::Reply::FinishedReason::ERROR:
//        {
//            reason = ListenerBase::Error;
//            err = req.getError();
//            break;
//        }
//        default:
//        {
//            assert(false);                // LCOV_EXCL_LINE
//            reason = ListenerBase::Error; // LCOV_EXCL_LINE
//        }
//    }
//    delegate->finished(reason, err);
}

void ReplyI::info_(Current const&,
                   capnp::AnyPointer::Reader& in_params,
                   capnproto::Response::Builder&)
{
    auto delegate = dynamic_pointer_cast<ReplyObjectBase>(del());
    auto req = in_params.getAs<capnproto::Reply::InfoRequest>();
    auto c = req.getCode();

    OperationInfo::InfoCode code = OperationInfo::Unknown;
    if (c >= 0 && c <= OperationInfo::LastInfoCode_)
    {
        code = static_cast<OperationInfo::InfoCode>(c);
    }

    delegate->info(OperationInfo{code, req.getMessage()});
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
