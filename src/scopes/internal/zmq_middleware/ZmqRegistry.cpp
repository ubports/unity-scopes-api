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

#include <unity/scopes/internal/zmq_middleware/ZmqRegistry.h>

#include <scopes/internal/zmq_middleware/capnproto/Registry.capnp.h>
#include <unity/scopes/internal/RegistryException.h>
#include <unity/scopes/internal/ScopeImpl.h>
#include <unity/scopes/internal/ScopeMetadataImpl.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqException.h>
#include <unity/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/scopes/ScopeExceptions.h>

#include <capnp/message.h>

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

interface Scope;

dictionary<string, ScopeMetadata> MetadataMap;

exception NotFoundException
{
    string scopeName;
};

interface Registry
{
    ScopeMetadata get_metadata(string name) throws NotFoundException;
    ScopeDict list();
    ScopeProxy string( name) throws NotFoundException, RegistryException;
};

*/

ZmqRegistry::ZmqRegistry(ZmqMiddleware* mw_base,
                         string const& endpoint,
                         string const& identity,
                         string const& category,
                         int64_t timeout) :
    MWObjectProxy(mw_base),
    ZmqObjectProxy(mw_base, endpoint, identity, category, RequestMode::Twoway, timeout),
    MWRegistry(mw_base)
{
}

ZmqRegistry::~ZmqRegistry()
{
}

ScopeMetadata ZmqRegistry::get_metadata(std::string const& scope_name)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "get_metadata");
    auto in_params = request.initInParams().getAs<capnproto::Registry::GetMetadataRequest>();
    in_params.setName(scope_name.c_str());

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    auto receiver = future.get();
    auto segments = receiver.receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto get_metadata_response = response.getPayload().getAs<capnproto::Registry::GetMetadataResponse>().getResponse();
    switch (get_metadata_response.which())
    {
        case capnproto::Registry::GetMetadataResponse::Response::RETURN_VALUE:
        {
            auto md = get_metadata_response.getReturnValue();
            VariantMap m = to_variant_map(md);
            unique_ptr<ScopeMetadataImpl> smdi(new ScopeMetadataImpl(m, mw_base()));
            return ScopeMetadata(ScopeMetadataImpl::create(move(smdi)));
        }
        case capnproto::Registry::GetMetadataResponse::Response::NOT_FOUND_EXCEPTION:
        {
            auto ex = get_metadata_response.getNotFoundException();
            throw NotFoundException("Registry::get_metadata(): no such scope", ex.getName().cStr());
        }
        default:
        {
            throw MiddlewareException("Registry::get_metadata(): unknown user exception");
        }
    }
}

MetadataMap ZmqRegistry::list()
{
    capnp::MallocMessageBuilder request_builder;
    make_request_(request_builder, "list");

    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder); });
    auto receiver = future.get();
    auto segments = receiver.receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto list_response = response.getPayload().getAs<capnproto::Registry::ListResponse>();
    auto list = list_response.getReturnValue().getPairs();
    MetadataMap sm;
    for (size_t i = 0; i < list.size(); ++i)
    {
        string scope_name = list[i].getName();
        VariantMap m = to_variant_map(list[i].getValue().getDictVal());
        unique_ptr<ScopeMetadataImpl> smdi(new ScopeMetadataImpl(m, mw_base()));
        ScopeMetadata d(ScopeMetadataImpl::create(move(smdi)));
        sm.emplace(make_pair(move(scope_name), move(d)));
    }
    return sm;
}

ScopeProxy ZmqRegistry::locate(std::string const& scope_name)
{
    capnp::MallocMessageBuilder request_builder;
    auto request = make_request_(request_builder, "locate");
    auto in_params = request.initInParams().getAs<capnproto::Registry::LocateRequest>();
    in_params.setName(scope_name.c_str());

    // locate uses a custom timeout because it needs to potentially fork/exec the scope.
    int64_t timeout = 1000; // TODO: get timeout from config
    auto future = mw_base()->invoke_pool()->submit([&] { return this->invoke_(request_builder, timeout); });
    auto receiver = future.get();
    auto segments = receiver.receive();
    capnp::SegmentArrayMessageReader reader(segments);
    auto response = reader.getRoot<capnproto::Response>();
    throw_if_runtime_exception(response);

    auto locate_response = response.getPayload().getAs<capnproto::Registry::LocateResponse>().getResponse();
    switch (locate_response.which())
    {
        case capnproto::Registry::LocateResponse::Response::RETURN_VALUE:
        {
            auto proxy = locate_response.getReturnValue();
            auto mw = dynamic_cast<ZmqMiddleware*>(mw_base());
            assert(mw);
            auto zmq_proxy = make_shared<ZmqScope>(mw,
                                                   proxy.getEndpoint(),
                                                   proxy.getIdentity(),
                                                   proxy.getCategory(),
                                                   proxy.getTimeout());
            return ScopeImpl::create(zmq_proxy, mw->runtime(), scope_name);
        }
        case capnproto::Registry::LocateResponse::Response::NOT_FOUND_EXCEPTION:
        {
            auto ex = locate_response.getNotFoundException();
            throw NotFoundException("Registry::locate(): no such scope", ex.getName().cStr());
        }
        case capnproto::Registry::LocateResponse::Response::REGISTRY_EXCEPTION:
        {
            auto ex = locate_response.getRegistryException();
            throw RegistryException(ex.getReason().cStr());
        }
        default:
        {
            throw MiddlewareException("Registry::locate(): unknown user exception");
        }
    }
}
} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
