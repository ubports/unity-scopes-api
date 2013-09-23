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

#include <unity/api/scopes/internal/zmq_middleware/ZmqRegistry.h>

#include <unity/api/scopes/internal/zmq_middleware/capnproto/Registry.capnp.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/api/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/api/scopes/internal/ScopeImpl.h>
#include <unity/api/scopes/ScopeExceptions.h>

#include <capnp/message.h>

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

interface Scope;

dictionary<string, Scope*> ScopeDictMap;

exception NotFoundException
{
    string scopeName;
};

interface Registry
{
    Scope* find(string name) throws NotFoundException;
    ScopeDict list();
};

*/

ZmqRegistry::ZmqRegistry(ZmqMiddleware* mw_base, string const& endpoint, string const& identity) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, RequestType::Twoway),
    MWRegistry(mw_base)
{
}

ZmqRegistry::~ZmqRegistry() noexcept
{
}

ScopeProxy ZmqRegistry::find(std::string const& scope_name)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "find");
    auto in_params = request.initInParams<capnproto::Registry::FindRequest>();
    in_params.setName(scope_name.c_str());

    auto future = mw_base()->invoke_pool()->submit([&]{ return this->invoke_(request_builder); });
    future.wait();
    auto receiver = future.get();
    auto segments = receiver->receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto find_response = response.getPayload<capnproto::Registry::FindResponse>().getResponse();
    switch (find_response.which())
    {
        case capnproto::Registry::FindResponse::Response::RETURN_VALUE:
        {
            auto proxy = find_response.getReturnValue();
            ZmqScopeProxy p(new ZmqScope(mw_base(), proxy.getEndpoint().cStr(), proxy.getIdentity().cStr()));
            return ScopeImpl::create(p, mw_base()->runtime());
        }
        case capnproto::Registry::FindResponse::Response::NOT_FOUND_EXCEPTION:
        {
            auto ex = find_response.getNotFoundException();
            throw NotFoundException("Registry::find(): no such scope", ex.getName().cStr());
        }
        default:
        {
            throw MiddlewareException("Registry::find(): unknown user exception");
        }
    }
}

ScopeMap ZmqRegistry::list()
{
    capnp::MallocMessageBuilder request_builder;
    make_request_(request_builder, "list");

    auto future = mw_base()->invoke_pool()->submit([&]{ return this->invoke_(request_builder); });
    future.wait();
    auto receiver = future.get();
    auto segments = receiver->receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto list_response = response.getPayload<capnproto::Registry::ListResponse>();
    auto pairs = list_response.getReturnValue().getPairs();
    ScopeMap sm;
    for (size_t i = 0; i < pairs.size(); ++i)
    {
        auto proxy = pairs[i].getScopeProxy();
        ZmqScopeProxy p(new ZmqScope(mw_base(), proxy.getEndpoint().cStr(), proxy.getIdentity().cStr()));
        sm[pairs[i].getName().cStr()] = ScopeImpl::create(p, mw_base()->runtime());
    }
    return sm;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
