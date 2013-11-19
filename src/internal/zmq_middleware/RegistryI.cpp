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

#include <scopes/internal/zmq_middleware/RegistryI.h>

#include <internal/zmq_middleware/capnproto/Registry.capnp.h>
#include <internal/zmq_middleware/capnproto/Scope.capnp.h>
#include <scopes/internal/zmq_middleware/ObjectAdapter.h>
#include <scopes/internal/zmq_middleware/ZmqScope.h>
#include <scopes/internal/RegistryObject.h>
#include <scopes/ScopeExceptions.h>

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

using namespace std::placeholders;

RegistryI::RegistryI(RegistryObject::SPtr const& ro) :
    ServantBase(ro, { { "find", bind(&RegistryI::find_, this, _1, _2, _3) },
                      { "list", bind(&RegistryI::list_, this, _1, _2, _3) } })

{
}

RegistryI::~RegistryI() noexcept
{
}

void RegistryI::find_(Current const&,
                      capnp::ObjectPointer::Reader& in_params,
                      capnproto::Response::Builder& r)
{
    auto req = in_params.getAs<capnproto::Registry::FindRequest>();
    string name = req.getName().cStr();
    auto delegate = dynamic_pointer_cast<RegistryObject>(del());
    try
    {
        auto proxy = dynamic_pointer_cast<ZmqObjectProxy>(delegate->find(name));
        assert(proxy);
        r.setStatus(capnproto::ResponseStatus::SUCCESS);
        auto find_response = r.initPayload().getAs<capnproto::Registry::FindResponse>().initResponse();
        auto p = find_response.initReturnValue();
        p.setEndpoint(proxy->endpoint().c_str());
        p.setIdentity(proxy->identity().c_str());
    }
    catch (NotFoundException const& e)
    {
        r.setStatus(capnproto::ResponseStatus::USER_EXCEPTION);
        auto find_response = r.initPayload().getAs<capnproto::Registry::FindResponse>().initResponse();
        find_response.initNotFoundException().setName(e.name().c_str());
    }
}

void RegistryI::list_(Current const&,
                      capnp::ObjectPointer::Reader&,
                      capnproto::Response::Builder& r)
{
    auto delegate = dynamic_pointer_cast<RegistryObject>(del());
    auto scope_map = delegate->list();
    r.setStatus(capnproto::ResponseStatus::SUCCESS);
    auto rv = r.initPayload().getAs<capnproto::Registry::ListResponse>().initReturnValue();
    auto list = rv.initPairs(scope_map.size());
    int i = 0;
    for (auto const& pair : scope_map)
    {
        list[i].setName(pair.first.c_str());
        auto p = list[i].initScopeProxy();
        auto sp = dynamic_pointer_cast<ZmqScope>(pair.second);
        p.setEndpoint(sp->endpoint().c_str());
        p.setIdentity(sp->identity().c_str());
        ++i;
    }
}

} // namespace zmq_middleware

} // namespace internal

} // namespace scopes

} // namespace api

} // namespace unity
