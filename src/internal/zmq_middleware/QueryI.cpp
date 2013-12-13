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

#include <scopes/internal/zmq_middleware/QueryI.h>

#include <internal/zmq_middleware/capnproto/Query.capnp.h>
#include <scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <scopes/internal/zmq_middleware/ZmqReply.h>
#include <scopes/internal/QueryObject.h>

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

interface Query
{
    void cancel();
    void destroy();
};

*/

using namespace std::placeholders;

QueryI::QueryI(QueryObject::SPtr const& qo) :
    ServantBase(qo, { { "run", bind(&QueryI::run_, this, _1, _2, _3) } })

{
}

QueryI::~QueryI() noexcept
{
}

void QueryI::run_(Current const& current,
                  capnp::ObjectPointer::Reader& in_params,
                  capnproto::Response::Builder&)
{
    auto req = in_params.getAs<capnproto::Query::RunRequest>();
    auto proxy = req.getReplyProxy();
    ZmqReplyProxy reply_proxy(new ZmqReply(current.adapter->mw(), proxy.getEndpoint().cStr(), proxy.getIdentity().cStr()));
    auto delegate = dynamic_pointer_cast<QueryObject>(del());
    delegate->run(reply_proxy);
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
