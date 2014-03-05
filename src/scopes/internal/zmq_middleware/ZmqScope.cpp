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

#include <unity/scopes/internal/zmq_middleware/ZmqScope.h>

#include <unity/scopes/internal/QueryCtrlImpl.h>
#include <scopes/internal/zmq_middleware/capnproto/Scope.capnp.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/CannedQuery.h>

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

dictionary<string, Value> ValueDict;

interface QueryCtrl;
interface Reply;

interface Scope
{
    QueryCtrl* search(string query, ValueDict hints, Reply* replyProxy);
    QueryCtrl* activate(string query, ValueDict hints, Reply* replyProxy);
    QueryCtrl* perform_action(string query, ValueDict hints, string action_id, Reply* replyProxy);
    QueryCtrl* preview(string query, ValueDict hints, Reply* replyProxy);
};

*/

ZmqScope::ZmqScope(ZmqMiddleware* mw_base,
                   string const& endpoint,
                   string const& identity,
                   string const& category,
                   int64_t timeout) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, category, RequestMode::Twoway, timeout),
    MWScope(mw_base)
{
}

ZmqScope::~ZmqScope()
{
}

QueryCtrlProxy ZmqScope::search(CannedQuery const& query, VariantMap const& hints, MWReplyProxy const& reply)
{
    capnp::MallocMessageBuilder request_builder;
    auto reply_proxy = dynamic_pointer_cast<ZmqReply>(reply);
    {
        auto request = make_request_(request_builder, "search");
        auto in_params = request.initInParams().getAs<capnproto::Scope::CreateQueryRequest>();
        auto q = in_params.initQuery();
        to_value_dict(query.serialize(), q);
        auto h = in_params.initHints();
        to_value_dict(hints, h);
        auto p = in_params.initReplyProxy();
        p.setEndpoint(reply_proxy->endpoint().c_str());
        p.setIdentity(reply_proxy->identity().c_str());
        p.setCategory(reply_proxy->category().c_str());
    }

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });

    auto receiver = future.get();
    auto segments = receiver.receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto proxy = response.getPayload().getAs<capnproto::Scope::CreateQueryResponse>().getReturnValue();
    ZmqQueryCtrlProxy p(new ZmqQueryCtrl(mw_base(),
                                         proxy.getEndpoint().cStr(),
                                         proxy.getIdentity().cStr(),
                                         proxy.getCategory().cStr()));
    return QueryCtrlImpl::create(p, reply_proxy);
}

QueryCtrlProxy ZmqScope::activate(VariantMap const& result, VariantMap const& hints, MWReplyProxy const& reply)
{
    capnp::MallocMessageBuilder request_builder;
    auto reply_proxy = dynamic_pointer_cast<ZmqReply>(reply);
    {
        auto request = make_request_(request_builder, "activate");
        auto in_params = request.initInParams().getAs<capnproto::Scope::ActivationRequest>();
        auto res = in_params.initResult();
        to_value_dict(result, res);
        auto h = in_params.initHints();
        to_value_dict(hints, h);
        auto p = in_params.initReplyProxy();
        p.setEndpoint(reply_proxy->endpoint().c_str());
        p.setIdentity(reply_proxy->identity().c_str());
    }

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });

    auto receiver = future.get();
    auto segments = receiver.receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto proxy = response.getPayload().getAs<capnproto::Scope::CreateQueryResponse>().getReturnValue();
    ZmqQueryCtrlProxy p(new ZmqQueryCtrl(mw_base(),
                                         proxy.getEndpoint().cStr(),
                                         proxy.getIdentity().cStr(),
                                         proxy.getCategory().cStr()));
    return QueryCtrlImpl::create(p, reply_proxy);
}

QueryCtrlProxy ZmqScope::perform_action(VariantMap const& result,
        VariantMap const& hints, std::string const& widget_id, std::string const& action_id, MWReplyProxy const& reply)
{
    capnp::MallocMessageBuilder request_builder;
    auto reply_proxy = dynamic_pointer_cast<ZmqReply>(reply);
    {
        auto request = make_request_(request_builder, "perform_action");
        auto in_params = request.initInParams().getAs<capnproto::Scope::ActionActivationRequest>();
        auto res = in_params.initResult();
        to_value_dict(result, res);
        auto h = in_params.initHints();
        to_value_dict(hints, h);
        in_params.setWidgetId(widget_id);
        in_params.setActionId(action_id);
        auto p = in_params.initReplyProxy();
        p.setEndpoint(reply_proxy->endpoint().c_str());
        p.setIdentity(reply_proxy->identity().c_str());
    }

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    future.wait();

    auto receiver = future.get();
    auto segments = receiver.receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto proxy = response.getPayload().getAs<capnproto::Scope::CreateQueryResponse>().getReturnValue();
    ZmqQueryCtrlProxy p(new ZmqQueryCtrl(mw_base(),
                                         proxy.getEndpoint().cStr(),
                                         proxy.getIdentity().cStr(),
                                         proxy.getCategory().cStr()));
    return QueryCtrlImpl::create(p, reply_proxy);
}

QueryCtrlProxy ZmqScope::preview(VariantMap const& result, VariantMap const& hints, MWReplyProxy const& reply)
{
    capnp::MallocMessageBuilder request_builder;
    auto reply_proxy = dynamic_pointer_cast<ZmqReply>(reply);
    {
        auto request = make_request_(request_builder, "preview");
        auto in_params = request.initInParams().getAs<capnproto::Scope::PreviewRequest>();
        auto res = in_params.initResult();
        to_value_dict(result, res);
        auto h = in_params.initHints();
        to_value_dict(hints, h);
        auto p = in_params.initReplyProxy();
        p.setEndpoint(reply_proxy->endpoint().c_str());
        p.setIdentity(reply_proxy->identity().c_str());
    }

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });

    auto receiver = future.get();
    auto segments = receiver.receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto proxy = response.getPayload().getAs<capnproto::Scope::CreateQueryResponse>().getReturnValue();
    ZmqQueryCtrlProxy p(new ZmqQueryCtrl(mw_base(),
                                         proxy.getEndpoint().cStr(),
                                         proxy.getIdentity().cStr(),
                                         proxy.getCategory().cStr()));
    return QueryCtrlImpl::create(p, reply_proxy);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
