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

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto::Registry");

using ScopeDict = import "ScopeDict.capnp";
using Proxy = import "Proxy.capnp";

# Registry interface
#
# Operations:
#
# Proxy find(string name) throws NotFoundException;
# ScopeDict list();

struct NotFoundException
{
    name @0 : Text;
}

struct FindRequest
{
    name @0 : Text;
}

struct FindResponse
{
    response : union
    {
        returnValue         @0 : Proxy.Proxy;
        notFoundException   @1 : NotFoundException;
    }
}

struct ListRequest
{
}

struct ListResponse
{
    returnValue @0 : ScopeDict.ScopeDict;
}
