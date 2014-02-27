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
#include <unity/scopes/internal/CannedQueryImpl.h>
#include <unity/scopes/internal/ScopeObject.h>
#include <unity/scopes/internal/ActionMetadataImpl.h>
#include <unity/scopes/internal/SearchMetadataImpl.h>
#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/scopes/CannedQuery.h>

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
    QueryCtrl* search(string query, ValueDict hints, Reply* replyProxy);
    QueryCtrl* preview(ValueDict result, ValueDict hints, Reply* replyProxy);
    QueryCtrl* perform_action(ValueDict result, ValueDict hints, string action_id, Reply* replyProxy);
    QueryCtrl* activate(ValueDict result, ValueDict hints, Reply* replyProxy);
};

*/

using namespace std::placeholders;

ScopeI::ScopeI(ScopeObjectBase::SPtr const& so) :
    ServantBase(so, {
        { "search", bind(&ScopeI::search_, this, _1, _2, _3) },
        { "preview", bind(&ScopeI::preview_, this, _1, _2, _3) },
        { "activate", bind(&ScopeI::activate_, this, _1, _2, _3) },
        { "perform_action", bind(&ScopeI::perform_action_, this, _1, _2, _3) }
    })
{
}

ScopeI::~ScopeI()
{
}

void ScopeI::search_(Current const& current,
                           capnp::AnyPointer::Reader& in_params,
                           capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Scope::CreateQueryRequest>();
    auto query = internal::CannedQueryImpl::create(to_variant_map(req.getQuery()));
    auto metadata = SearchMetadataImpl::create(to_variant_map(req.getHints()));
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                              proxy.getEndpoint().cStr(),
                              proxy.getIdentity().cStr(),
                              proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->search(query,
                                                                                metadata,
                                                                                reply_proxy,
                                                                                to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto search_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = search_response.initReturnValue();
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
    auto metadata = ActionMetadataImpl::create(to_variant_map(req.getHints()));
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                                           proxy.getEndpoint().cStr(),
                                           proxy.getIdentity().cStr(),
                                           proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->activate(result,
                                                                            metadata,
                                                                            reply_proxy,
                                                                            to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto search_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = search_response.initReturnValue();
    p.setEndpoint(ctrl_proxy->endpoint().c_str());
    p.setIdentity(ctrl_proxy->identity().c_str());
    p.setCategory(ctrl_proxy->category().c_str());
}

void ScopeI::perform_action_(Current const& current,
                           capnp::AnyPointer::Reader& in_params,
                           capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Scope::ActionActivationRequest>();
    auto result = ResultImpl::create_result(to_variant_map(req.getResult()));
    auto metadata = ActionMetadataImpl::create(to_variant_map(req.getHints()));
    auto proxy = req.getReplyProxy();
    auto widget_id = req.getWidgetId().cStr();
    auto action_id = req.getActionId().cStr();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                                           proxy.getEndpoint().cStr(),
                                           proxy.getIdentity().cStr(),
                                           proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->perform_action(result,
                                                                                  metadata,
                                                                                  widget_id,
                                                                                  action_id,
                                                                                  reply_proxy,
                                                                                  to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto search_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = search_response.initReturnValue();
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
    auto metadata = ActionMetadataImpl::create(to_variant_map(req.getHints()));
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(),
                                           proxy.getEndpoint().cStr(),
                                           proxy.getIdentity().cStr(),
                                           proxy.getCategory().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->preview(result,
                                                                           metadata,
                                                                           reply_proxy,
                                                                           to_info(current)));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto search_response = r.initPayload().getAs<capnproto::Scope::CreateQueryResponse>();
    auto p = search_response.initReturnValue();
    p.setEndpoint(ctrl_proxy->endpoint().c_str());
    p.setIdentity(ctrl_proxy->identity().c_str());
    p.setCategory(ctrl_proxy->category().c_str());
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
