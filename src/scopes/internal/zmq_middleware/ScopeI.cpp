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

#include <unity/scopes/internal/zmq_middleware/ScopeI.h>

#include <scopes/internal/zmq_middleware/capnproto/Scope.capnp.h>
#include <unity/scopes/internal/ResultImpl.h>
#include <unity/scopes/internal/QueryImpl.h>
#include <unity/scopes/internal/ScopeObject.h>
#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/scopes/Query.h>

#include <cassert>

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
    QueryCtrl* create_query(string query, ValueDict hints, Reply* replyProxy);
    QueryCtrl* preview(ValueDict result, ValueDict hints, Reply* replyProxy);
    QueryCtrl* activate_preview_action(ValueDict result, ValueDict hints, string action_id, Reply* replyProxy);
    QueryCtrl* activate(ValueDict result, ValueDict hints, Reply* replyProxy);
};

*/

using namespace std::placeholders;

ScopeI::ScopeI(ScopeObjectBase::SPtr const& so) :
    ServantBase(so, {
        { "create_query", bind(&ScopeI::create_query_, this, _1, _2, _3) },
        { "preview", bind(&ScopeI::preview_, this, _1, _2, _3) },
        { "activate", bind(&ScopeI::activate_, this, _1, _2, _3) },
        { "activate_preview_action", bind(&ScopeI::activate_preview_action_, this, _1, _2, _3) }
    })
{
}

ScopeI::~ScopeI()
{
}

void ScopeI::create_query_(Current const& current,
                           capnp::AnyPointer::Reader& in_params,
                           capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Scope::CreateQueryRequest>();
    auto query = internal::QueryImpl::create(to_variant_map(req.getQuery()));
    auto hints = to_variant_map(req.getHints());
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                              proxy.getEndpoint().cStr(),
                              proxy.getIdentity().cStr(),
                              proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->create_query(query,
                                                                                hints,
                                                                                reply_proxy,
                                                                                to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto create_query_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = create_query_response.initReturnValue();
    p.setEndpoint(ctrl_proxy->endpoint().c_str());
    p.setIdentity(ctrl_proxy->identity().c_str());
    p.setCategory(ctrl_proxy->category().c_str());
}

void ScopeI::activate_(Current const& current,
               capnp::AnyPointer::Reader& in_params,
               capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Scope::ActivationRequest>();
    auto result = ResultImpl::create_result(to_variant_map(req.getResult()));
    auto hints = to_variant_map(req.getHints());
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                                           proxy.getEndpoint().cStr(),
                                           proxy.getIdentity().cStr(),
                                           proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->activate(result,
                                                                            hints,
                                                                            reply_proxy,
                                                                            to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto create_query_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = create_query_response.initReturnValue();
    p.setEndpoint(ctrl_proxy->endpoint().c_str());
    p.setIdentity(ctrl_proxy->identity().c_str());
    p.setCategory(ctrl_proxy->category().c_str());
}

void ScopeI::activate_preview_action_(Current const& current,
                           capnp::AnyPointer::Reader& in_params,
                           capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Scope::ActionActivationRequest>();
    auto result = ResultImpl::create_result(to_variant_map(req.getResult()));
    auto hints = to_variant_map(req.getHints());
    auto proxy = req.getReplyProxy();
    auto action_id = req.getAction().cStr();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                                           proxy.getEndpoint().cStr(),
                                           proxy.getIdentity().cStr(),
                                           proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObject>(del());
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->activate_preview_action(result,
                                                                                           hints,
                                                                                           action_id,
                                                                                           reply_proxy,
                                                                                           to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto create_query_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = create_query_response.initReturnValue();
    p.setEndpoint(ctrl_proxy->endpoint().c_str());
    p.setIdentity(ctrl_proxy->identity().c_str());
    p.setCategory(ctrl_proxy->category().c_str());
}

void ScopeI::preview_(Current const& current,
                      capnp::AnyPointer::Reader& in_params,
                      capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Scope::PreviewRequest>();
    auto result = ResultImpl::create_result(to_variant_map(req.getResult()));
    auto hints = to_variant_map(req.getHints());
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                                           proxy.getEndpoint().cStr(),
                                           proxy.getIdentity().cStr(),
                                           proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->preview(result,
                                                                           hints,
                                                                           reply_proxy,
                                                                           to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto create_query_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = create_query_response.initReturnValue();
    p.setEndpoint(ctrl_proxy->endpoint().c_str());
    p.setIdentity(ctrl_proxy->identity().c_str());
    p.setCategory(ctrl_proxy->category().c_str());
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
