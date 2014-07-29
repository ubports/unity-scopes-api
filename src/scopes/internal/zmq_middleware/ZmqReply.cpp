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

#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>
#include <scopes/internal/zmq_middleware/capnproto/Reply.capnp.h>

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
    void push(VariantMap result);                     // oneway
    void finished(CompletionDetails const& details);  // oneway
};

*/

ZmqReply::ZmqReply(ZmqMiddleware* mw_base, string const& endpoint, string const& identity, string const& category) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, category, RequestMode::Oneway),
    MWReply(mw_base)
{
}

ZmqReply::~ZmqReply()
{
}

void ZmqReply::push(VariantMap const& result)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "push");
    auto in_params = request.initInParams().getAs<capnproto::Reply::PushRequest>();

    auto resultBuilder = in_params.getResult();
    to_value_dict(result, resultBuilder);

    auto future = mw_base()->oneway_pool()->submit([&] { return this->invoke_oneway_(request_builder); });
    future.wait();
}

void ZmqReply::finished(CompletionDetails const& details)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "finished");
    auto in_params = request.initInParams().getAs<capnproto::Reply::FinishedRequest>();
    capnproto::Reply::CompletionStatus s;
    switch (details.status())
    {
        case CompletionDetails::OK:
        {
            s = capnproto::Reply::CompletionStatus::OK;
            break;
        }
        case CompletionDetails::Cancelled:
        {
            s = capnproto::Reply::CompletionStatus::CANCELLED;
            break;
        }
        case CompletionDetails::Error:
        {
            s = capnproto::Reply::CompletionStatus::ERROR;
            break;
        }
        default:
        {
            assert(false);
            s = capnproto::Reply::CompletionStatus::ERROR;
        }
    }
    in_params.setStatus(s);
    in_params.setMessage(details.message());

    auto future = mw_base()->oneway_pool()->submit([&] { return this->invoke_oneway_(request_builder); });
    future.wait();
}

void ZmqReply::info(OperationInfo const& op_info)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "info");
    auto in_params = request.initInParams().getAs<capnproto::Reply::InfoRequest>();

    in_params.setCode(static_cast<int16_t>(op_info.code()));
    in_params.setMessage(op_info.message());

    auto future = mw_base()->oneway_pool()->submit([&] { return this->invoke_oneway_(request_builder); });
    future.wait();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
