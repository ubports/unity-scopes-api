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
#include <internal/zmq_middleware/capnproto/Scope.capnp.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqQueryCtrl.h>
#include <unity/scopes/internal/zmq_middleware/ZmqReply.h>

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
};

*/

ZmqScope::ZmqScope(ZmqMiddleware* mw_base, string const& endpoint, string const& identity) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, RequestType::Twoway),
    MWScope(mw_base)
{
}

ZmqScope::~ZmqScope() noexcept
{
}

QueryCtrlProxy ZmqScope::create_query(std::string const& q, VariantMap const& hints, MWReplyProxy const& reply)
{
    capnp::MallocMessageBuilder request_builder;
    auto reply_proxy = dynamic_pointer_cast<ZmqReply>(reply);
    {
        auto request = make_request_(request_builder, "create_query");
        auto in_params = request.initInParams().getAs<capnproto::Scope::CreateQueryRequest>();
        in_params.setQuery(q.c_str());
        auto h = in_params.initHints();
        to_value_dict(hints, h);
        auto p = in_params.initReplyProxy();
        p.setEndpoint(reply_proxy->endpoint().c_str());
        p.setIdentity(reply_proxy->identity().c_str());
    }

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    future.wait();

    auto receiver = future.get();
    auto segments = receiver->receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto proxy = response.getPayload().getAs<capnproto::Scope::CreateQueryResponse>().getReturnValue();
    ZmqQueryCtrlProxy p(new ZmqQueryCtrl(mw_base(), proxy.getEndpoint().cStr(), proxy.getIdentity().cStr()));
    return QueryCtrlImpl::create(p, reply_proxy);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
