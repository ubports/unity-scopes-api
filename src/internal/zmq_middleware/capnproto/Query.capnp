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

@0xa2f426b0db4141c5;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::scopes::internal::zmq_middleware::capnproto::Query");

using Proxy = import "Proxy.capnp";

# Query interface
#
# The separate Query object solely exists so we can control a query
# without the risk of blocking. The Scope::createQuery() method is on the -ctrl
# endpoint, whereas the Query object is on the normal endpoint. This allows
# createQuery() to start the query running by calling run(), which causes run()
# to be dispatched in a thread from the normal endpoint, and createQuery() is guaranteed
# to not block.
#
# Operations:
#
# void run(Proxy reply_proxy);

struct RunRequest
{
    replyProxy @0 : Proxy.Proxy;
}

struct RunResponse
{
}
