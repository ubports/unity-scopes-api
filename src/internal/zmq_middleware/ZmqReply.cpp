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

#include <scopes/internal/zmq_middleware/ZmqReply.h>
#include <scopes/internal/zmq_middleware/VariantConverter.h>
#include <internal/zmq_middleware/capnproto/Reply.capnp.h>

#include <scopes/ResultItem.h>
#include <scopes/internal/ResultItemImpl.h>

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

interface Reply
{
    void push(string result);   // oneway
    void finished();            // oneway
};

*/

ZmqReply::ZmqReply(ZmqMiddleware* mw_base, string const& endpoint, string const& identity) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, RequestType::Oneway),
    MWReply(mw_base)
{
}

ZmqReply::~ZmqReply() noexcept
{
}

void ZmqReply::push(std::shared_ptr<VariantMap> result)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "push");
    auto in_params = request.initInParams().getAs<capnproto::Reply::PushRequest>();
    auto resultBuilder = in_params.getResult();
    to_value_dict(*result, resultBuilder);

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    future.wait();
}

void ZmqReply::finished()
{
    capnp::MallocMessageBuilder request_builder;
    make_request_(request_builder, "finished");

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    future.wait();
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
