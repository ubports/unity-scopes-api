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

void ZmqReply::warning(Reply::Warning w, std::string const& warning_message)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "warning");
    auto in_params = request.initInParams().getAs<capnproto::Reply::WarningRequest>();
    switch (w)
    {
        case Reply::NoInternetConnection:
        {
            in_params.setWarning(capnproto::Reply::Warning::NO_INTERNET_CONNECTION);
            break;
        }
        case Reply::PoorInternetConnection:
        {
            in_params.setWarning(capnproto::Reply::Warning::POOR_INTERNET_CONNECTION);
            break;
        }
        case Reply::NoLocationData:
        {
            in_params.setWarning(capnproto::Reply::Warning::NO_LOCATION_DATA);
            break;
        }
        case Reply::InaccurateLocationData:
        {
            in_params.setWarning(capnproto::Reply::Warning::INACCURATE_LOCATION_DATA);
            break;
        }
        default:
        {
            assert(false); // LCOV_EXCL_LINE
            return;        // LCOV_EXCL_LINE
        }
    }
    in_params.setMessage(warning_message);

    auto future = mw_base()->oneway_pool()->submit([&] { return this->invoke_oneway_(request_builder); });
    future.wait();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
