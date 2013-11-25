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

#include <scopes/internal/zmq_middleware/ZmqQuery.h>

#include <internal/zmq_middleware/capnproto/Query.capnp.h>
#include <scopes/internal/zmq_middleware/ZmqReply.h>

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

interface Query
{
    void run(Reply* r);
};

*/

ZmqQuery::ZmqQuery(ZmqMiddleware* mw_base, string const& endpoint, string const& identity) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, RequestType::Oneway),
    MWQuery(mw_base)
{
}

ZmqQuery::~ZmqQuery() noexcept
{
}

void ZmqQuery::run(MWReplyProxy const& reply)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "run");
    auto in_params = request.initInParams().getAs<capnproto::Query::RunRequest>();
    auto proxy = in_params.initReplyProxy();
    auto rp = dynamic_pointer_cast<ZmqReply>(reply);
    proxy.setEndpoint(rp->endpoint().c_str());
    proxy.setIdentity(rp->identity().c_str());

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    future.wait();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
