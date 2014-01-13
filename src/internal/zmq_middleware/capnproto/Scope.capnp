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

@0xab8a5db476d65443;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto::Scope");

using Proxy = import "Proxy.capnp";
using ValueDict = import "ValueDict.capnp";

# Factory for queries. The Scope object runs on the ctrl-endpoint.
# createQuery() creates a Query object on the normal endpoint, and a QueryCtrl object
# on the ctrl-endpoint. It then calls run() on the Query object to set it executing in its own thread, before
# returning the QueryCtrl proxy that permits cancellation. This guarantees that createQuery() will not block.

# Scope interface
#
# Operations:
#
# Proxy createQuery(string query, ValueDict hints, Proxy reply_proxy);

# The createQuery method instantiates a Query Object and its corresponding QueryCtrlObject.
# The return value is the proxy to the QueryCtrl object.
# The implementation of createQuery calls the Query's run() method to give a thread of control
# to the application code.

struct CreateQueryRequest
{
    query @0      : Text;
    hints @1      : ValueDict.ValueDict;
    replyProxy @2 : Proxy.Proxy;
}

struct CreateQueryResponse
{
    returnValue @0 : Proxy.Proxy;
}

struct ActivationRequest
{
    result @0     : ValueDict.ValueDict;
    hints @1      : ValueDict.ValueDict;
    replyProxy @2 : Proxy.Proxy;
}

struct PreviewRequest
{
    result @0     : ValueDict.ValueDict;
    hints @1      : ValueDict.ValueDict;
    replyProxy @2 : Proxy.Proxy;
}
