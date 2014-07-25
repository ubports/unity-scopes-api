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
    void push(VariantMap result);                                      // oneway
    void finished(ListenerBase::Reason reason, string error_message);  // oneway
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

void ZmqReply::finished(ListenerBase::Reason reason, string const& error_message)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "finished");
    auto in_params = request.initInParams().getAs<capnproto::Reply::FinishedRequest>();
    capnproto::Reply::FinishedReason r;
    switch (reason)
    {
        case ListenerBase::Finished:
        {
            r = capnproto::Reply::FinishedReason::FINISHED;
            break;
        }
        case ListenerBase::Cancelled:
        {
            r = capnproto::Reply::FinishedReason::CANCELLED;
            break;
        }
        case ListenerBase::Error:
        {
            r = capnproto::Reply::FinishedReason::ERROR;
            in_params.setError(error_message);
            break;
        }
        default:
        {
            assert(false);
        }
    }
    in_params.setReason(r);

    auto future = mw_base()->oneway_pool()->submit([&] { return this->invoke_oneway_(request_builder); });
    future.wait();
}

void ZmqReply::info(OperationInfo const& op_info)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "info");
    auto in_params = request.initInParams().getAs<capnproto::Reply::InfoRequest>();
    switch (op_info.code())
    {
        case OperationInfo::NoInternet:
        {
            in_params.setCode(capnproto::Reply::InfoCode::NO_INTERNET);
            break;
        }
        case OperationInfo::PoorInternet:
        {
            in_params.setCode(capnproto::Reply::InfoCode::POOR_INTERNET);
            break;
        }
        case OperationInfo::NoLocationData:
        {
            in_params.setCode(capnproto::Reply::InfoCode::NO_LOCATION_DATA);
            break;
        }
        case OperationInfo::InaccurateLocationData:
        {
            in_params.setCode(capnproto::Reply::InfoCode::INACCURATE_LOCATION_DATA);
            break;
        }
        case OperationInfo::ResultsIncomplete:
        {
            in_params.setCode(capnproto::Reply::InfoCode::RESULTS_INCOMPLETE);
            break;
        }
        case OperationInfo::DefaultSettingsUsed:
        {
            in_params.setCode(capnproto::Reply::InfoCode::DEFAULT_SETTINGS_USED);
            break;
        }
        case OperationInfo::SettingsProblem:
        {
            in_params.setCode(capnproto::Reply::InfoCode::SETTINGS_PROBLEM);
            break;
        }
        default:
        {
            in_params.setCode(capnproto::Reply::InfoCode::UNKNOWN);
        }
    }
    in_params.setMessage(op_info.message());

    auto future = mw_base()->oneway_pool()->submit([&] { return this->invoke_oneway_(request_builder); });
    future.wait();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
