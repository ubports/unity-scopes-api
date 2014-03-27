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

#include <unity/scopes/internal/zmq_middleware/RegistryI.h>

#include <scopes/internal/zmq_middleware/capnproto/Registry.capnp.h>
#include <scopes/internal/zmq_middleware/capnproto/Scope.capnp.h>
#include <unity/scopes/internal/RegistryException.h>
#include <unity/scopes/internal/RegistryObjectBase.h>
#include <unity/scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <unity/scopes/internal/zmq_middleware/VariantConverter.h>
#include <unity/scopes/internal/zmq_middleware/ZmqScope.h>
#include <unity/scopes/ScopeExceptions.h>

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

interface Scope;

dictionary<string, VariantMap> MetadataMap;

exception NotFoundException
{
    string scopeName;
};

interface Registry
{
    ScopeProxy get_metadata(string name) throws NotFoundException;
    ScopeMap list();
    ScopeProxy locate(string name) throws NotFoundException, RegistryException;
};

*/

using namespace std::placeholders;

RegistryI::RegistryI(RegistryObjectBase::SPtr const& ro) :
    ServantBase(ro, { { "get_metadata", bind(&RegistryI::get_metadata_, this, _1, _2, _3) },
                      { "list", bind(&RegistryI::list_, this, _1, _2, _3) },
                      { "locate", bind(&RegistryI::locate_, this, _1, _2, _3) } })

{
}

RegistryI::~RegistryI()
{
}

void RegistryI::get_metadata_(Current const&,
                              capnp::AnyPointer::Reader& in_params,
                              capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Registry::GetMetadataRequest>();
    string name = req.getName().cStr();
    auto delegate = dynamic_pointer_cast<RegistryObjectBase>(del());
    try
    {
        auto meta = delegate->get_metadata(name);
        r.setStatus(capnproto::ResponseStatus::SUCCESS);
        auto get_metadata_response = r.initPayload().getAs<capnproto::Registry::GetMetadataResponse>().initResponse();
        auto dict = get_metadata_response.initReturnValue();
        to_value_dict(meta.serialize(), dict);
    }
    catch (NotFoundException const& e)
    {
        r.setStatus(capnproto::ResponseStatus::USER_EXCEPTION);
        auto get_metadata_response = r.initPayload().getAs<capnproto::Registry::GetMetadataResponse>().initResponse();
        get_metadata_response.initNotFoundException().setName(e.name().c_str());
    }
}

void RegistryI::list_(Current const&,
                      capnp::AnyPointer::Reader&,
                      capnproto::Response::Builder& r)
{
    auto delegate = dynamic_pointer_cast<RegistryObjectBase>(del());
    auto metadata_map = delegate->list();
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto list_response = r.initPayload().getAs<capnproto::Registry::ListResponse>();
    auto dict = list_response.initReturnValue().initPairs(metadata_map.size());
    int i = 0;
    for (auto& pair : metadata_map)
    {
        dict[i].setName(pair.first.c_str());            // Scope name
        auto md = dict[i].initValue().initDictVal();
        to_value_dict(pair.second.serialize(), md);     // Scope metadata
        ++i;
    }
}

void RegistryI::locate_(Current const&,
                        capnp::AnyPointer::Reader& in_params,
                        capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Registry::GetMetadataRequest>();
    string name = req.getName().cStr();
    auto delegate = dynamic_pointer_cast<RegistryObjectBase>(del());
    try
    {
        auto scope_proxy = delegate->locate(name);
        r.setStatus(capnproto::ResponseStatus::SUCCESS);
        auto locate_response = r.initPayload().getAs<capnproto::Registry::LocateResponse>().initResponse();
        auto proxy = locate_response.initReturnValue();
        proxy.setIdentity(scope_proxy->identity());
        proxy.setCategory("Scope");
        proxy.setEndpoint(scope_proxy->endpoint());
        proxy.setTimeout(scope_proxy->timeout());
    }
    catch (NotFoundException const& e)
    {
        r.setStatus(capnproto::ResponseStatus::USER_EXCEPTION);
        auto locate_response = r.initPayload().getAs<capnproto::Registry::LocateResponse>().initResponse();
        locate_response.initNotFoundException().setName(e.name().c_str());
    }
    catch (RegistryException const& e)
    {
        r.setStatus(capnproto::ResponseStatus::USER_EXCEPTION);
        auto locate_response = r.initPayload().getAs<capnproto::Registry::LocateResponse>().initResponse();
        locate_response.initRegistryException().setReason(e.reason().c_str());
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace unity
