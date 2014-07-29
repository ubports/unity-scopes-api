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
{
    auto delegate = dynamic_pointer_cast<ReplyObjectBase>(del());
    auto req = in_params.getAs<capnproto::Reply::FinishedRequest>();
    auto s = req.getStatus();
    CompletionDetails::CompletionStatus status;
    string msg;
    switch (s)
    {
        case capnproto::Reply::CompletionStatus::OK:
        {
            status = CompletionDetails::OK;
            msg = req.getMessage();
            break;
        }
        case capnproto::Reply::CompletionStatus::CANCELLED:
        {
            status = CompletionDetails::Cancelled;
            msg = req.getMessage();
            break;
        }
        case capnproto::Reply::CompletionStatus::ERROR:
        {
            status = CompletionDetails::Error;
            msg = req.getMessage();
            break;
        }
        default:
        {
            assert(false);                     // LCOV_EXCL_LINE
            status = CompletionDetails::Error; // LCOV_EXCL_LINE
        }
    }
    delegate->finished(CompletionDetails(status, msg));
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
