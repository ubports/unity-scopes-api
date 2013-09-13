/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <unity/api/scopes/internal/zmq_middleware/ScopeI.h>

#include <unity/api/scopes/internal/zmq_middleware/capnproto/Scope.capnp.h>
#include <unity/api/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/api/scopes/internal/zmq_middleware/VariantConverter.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqReply.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/api/scopes/internal/ScopeObject.h>

#include <cassert>

using namespace std;

namespace unity
{

namespace api
{

namespace scopes
{

namespace internal
{

namespace zmq_middleware
{

/*

enum ValueType { IntType, BoolType, StringType };

class ValueBase
{
    ValueType type;
};

dictionary<string, ValueBase> ValueDict;

class IntValue extends ValueBase
{
    int value;
};

class BoolValue extends ValueBase
{
    bool value;
};

class StringValue extends ValueBase
{
    string value;
};

interface QueryCtrl;
interface Reply;

interface Scope
{
    QueryCtrl* createQuery(string q, ValueDict hints, Reply* r);
};

*/

using namespace std::placeholders;

ScopeI::ScopeI(ScopeObject::SPtr const& so) :
    ServantBase(so, { { "create_query", bind(&ScopeI::create_query_, this, _1, _2, _3) } })

{
}

ScopeI::~ScopeI() noexcept
{
}

void ScopeI::create_query_(Current const& current,
                      capnp::DynamicObject::Reader& in_params,
                      capnproto::Response::Builder& r)
{
    auto req = in_params.as<capnproto::Scope::CreateQueryRequest>();
    auto query = req.getQuery().cStr();
    auto hints = to_variant_map(req.getHints());
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(), proxy.getEndpoint().cStr(), proxy.getIdentity().cStr()));
    auto delegate = dynamic_pointer_cast<ScopeObject>(del());
    auto ctrl_proxy = dynamic_pointer_cast<ZmqQueryCtrl>(delegate->create_query(query, hints, reply_proxy, current.adapter->mw()));
    assert(ctrl_proxy);
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto create_query_response = r.initPayload<capnproto::Scope::CreateQueryResponse>();
    auto p = create_query_response.initReturnValue();
    p.setEndpoint(ctrl_proxy->endpoint().c_str());
    p.setIdentity(ctrl_proxy->identity().c_str());
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
