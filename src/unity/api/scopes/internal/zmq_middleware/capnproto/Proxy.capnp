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

@0xb26286dd0ff9410f;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto");

# Proxy definition. A proxy is a pair of <endpoint, identity>. The endpoint must include
# the protocol, which controls how to interpret the remainder of the string. For example:
#
# "tcp://localhost:5555" (TCP on loopback, port 5555
#
# "ipc:///tmp/socket" (UNIX domain socket at /tmp/socket)
#
# The identity can be anything. It must be unique withing the corresponding endpoint. The identity
# determines the target object of a message sent via the proxy.

struct Proxy
{
    endpoint @0 : Text;
    identity @1 : Text;
}
