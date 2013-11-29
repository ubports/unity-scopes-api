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

#include <scopes/internal/zmq_middleware/ZmqRegistry.h>

#include <internal/zmq_middleware/capnproto/Registry.capnp.h>
#include <scopes/internal/zmq_middleware/VariantConverter.h>
#include <scopes/internal/zmq_middleware/ZmqException.h>
#include <scopes/internal/zmq_middleware/ZmqScope.h>
#include <scopes/internal/ScopeImpl.h>
#include <scopes/internal/ScopeMetadataImpl.h>
#include <scopes/ScopeExceptions.h>

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

dictionary<string, ScopeMetadata> ScopeDictMap;

exception NotFoundException
{
    string scopeName;
};

interface Registry
{
    ScopeMetadata find(string name) throws NotFoundException;
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

ScopeMetadata ZmqRegistry::find(std::string const& scope_name)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "find");
    auto in_params = request.initInParams().getAs<capnproto::Registry::FindRequest>();
    in_params.setName(scope_name.c_str());

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    future.wait();
    auto receiver = future.get();
    auto segments = receiver->receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto find_response = response.getPayload().getAs<capnproto::Registry::FindResponse>().getResponse();
    switch (find_response.which())
    {
        case capnproto::Registry::FindResponse::Response::RETURN_VALUE:
        {
            auto md = find_response.getReturnValue();
            VariantMap m = to_variant_map(md);
            unique_ptr<ScopeMetadataImpl> smdi(new ScopeMetadataImpl(mw_base()));
            smdi->deserialize(m);
            return ScopeMetadata(ScopeMetadataImpl::create(move(smdi)));
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

MetadataMap ZmqRegistry::list()
{
    capnp::MallocMessageBuilder request_builder;
    make_request_(request_builder, "list");

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    future.wait();
    auto receiver = future.get();
    auto segments = receiver->receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto list_response = response.getPayload().getAs<capnproto::Registry::ListResponse>();
    auto list = list_response.getReturnValue();
    MetadataMap sm;
    for (size_t i = 0; i < list.size(); ++i)
    {
        VariantMap m = to_variant_map(list[i]);
        string scope_name = m["scope_name"].get_string();
        unique_ptr<ScopeMetadataImpl> smdi(new ScopeMetadataImpl(mw_base()));
        smdi->deserialize(m);
        ScopeMetadata d(ScopeMetadataImpl::create(move(smdi)));
        sm.emplace(make_pair(move(scope_name), move(d)));
    }
    return sm;
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
