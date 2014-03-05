#
# Copyright (C) 2013 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Michi Henning <michi.henning@canonical.com>
#

@0xb1b042af3cad4891;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::scopes::internal::zmq_middleware::capnproto::Registry");

using ValueDict = import "ValueDict.capnp";
using Proxy = import "Proxy.capnp";

# Registry interface
#
# Operations:
#
# ValueDict get_metadata(string name) throws NotFoundException;
# map<string, ScopeMetadata> list();
# ScopeProxy locate(string name) throws NotFoundException, RegistryException;

struct NotFoundException
{
    name @0 : Text;
}

struct RegistryException
{
    reason @0 : Text;
}

struct GetMetadataRequest
{
    name @0 : Text;
}

struct GetMetadataResponse
{
    response : union
    {
        returnValue         @0 : ValueDict.ValueDict;
        notFoundException   @1 : NotFoundException;
    }
}

struct ListRequest
{
}

struct ListResponse
{
    returnValue @0 : ValueDict.ValueDict;   # Dictionary of dictionaries: <scope_id, ScopeMetadata>
}

struct LocateRequest
{
    name @0 : Text;
}

struct LocateResponse
{
    response : union
    {
        returnValue         @0 : Proxy.Proxy;
        notFoundException   @1 : NotFoundException;
        registryException   @2 : RegistryException;
    }
}
