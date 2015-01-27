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
    ChildScopeList child_scopes_ordered();
    bool set_child_scopes_ordered(ChildScopeList const& child_scopes_ordered);
    bool debug_mode();
};

*/

using namespace std;
namespace ph = std::placeholders;

ScopeI::ScopeI(ScopeObjectBase::SPtr const& so) :
    ServantBase(so, { { "search", bind(&ScopeI::search_, this, ph::_1, ph::_2, ph::_3) },
                      { "preview", bind(&ScopeI::preview_, this, ph::_1, ph::_2, ph::_3) },
                      { "activate", bind(&ScopeI::activate_, this, ph::_1, ph::_2, ph::_3) },
                      { "perform_action", bind(&ScopeI::perform_action_, this, ph::_1, ph::_2, ph::_3) },
                      { "child_scopes_ordered", bind(&ScopeI::child_scopes_ordered_, this, ph::_1, ph::_2, ph::_3) },
                      { "set_child_scopes_ordered", bind(&ScopeI::set_child_scopes_ordered_, this, ph::_1, ph::_2, ph::_3) },
                      { "debug_mode", bind(&ScopeI::debug_mode_, this, ph::_1, ph::_2, ph::_3) }
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
    p.setCategory(ctrl_proxy->target_category().c_str());
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
    p.setCategory(ctrl_proxy->target_category().c_str());
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
    p.setCategory(ctrl_proxy->target_category().c_str());
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
    p.setCategory(ctrl_proxy->target_category().c_str());
}

void ScopeI::child_scopes_ordered_(Current const&,
                      capnp::AnyPointer::Reader&,
                      capnproto::Response::Builder& r)
{
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);

    auto child_scopes_ordered = delegate->child_scopes_ordered();

    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto list_response = r.initPayload().getAs<capnproto::Scope::ChildScopesOrderedResponse>();
    auto list = list_response.initReturnValue(child_scopes_ordered.size());

    int i = 0;
    for (auto const& child_scope : child_scopes_ordered)
    {
        list[i].setId(child_scope.id);
        list[i].setEnabled(child_scope.enabled);
        ++i;
    }
}

void ScopeI::set_child_scopes_ordered_(Current const&,
                      capnp::AnyPointer::Reader& in_params,
                      capnproto::Response::Builder& r)
{
    auto delegate = std::dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);

    auto list = in_params.getAs<capnproto::Scope::SetChildScopesOrderedRequest>().getChildScopesOrdered();

    ChildScopeList child_scope_list;
    for (size_t i = 0; i < list.size(); ++i)
    {
        string id = list[i].getId();
        bool enabled = list[i].getEnabled();
        child_scope_list.push_back( ChildScope{id, enabled} );
    }

    bool result = delegate->set_child_scopes_ordered(child_scope_list);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto response = r.initPayload().getAs<capnproto::Scope::SetChildScopesOrderedResponse>();
    response.setReturnValue(result);
}

void ScopeI::debug_mode_(Current const&,
                         capnp::AnyPointer::Reader&,
                         capnproto::Response::Builder& r)
{
    auto delegate = dynamic_pointer_cast<ScopeObjectBase>(del());
    assert(delegate);
    auto debug_mode = delegate->debug_mode();
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto debug_mode_response = r.initPayload().getAs<capnproto::Scope::DebugModeResponse>();
    debug_mode_response.setReturnValue(debug_mode);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
