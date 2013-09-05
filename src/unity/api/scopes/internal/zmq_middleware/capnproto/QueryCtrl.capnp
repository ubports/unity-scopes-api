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

@0x9903f9de544b6365;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto::QueryCtrl");

# The separate QueryCtrl object solely exists so we can cancel a query
# without the risk of blocking. The QueryCtrl::cancel() method is on the -ctrl
# endpoint, whereas the Query object is on the normal endpoint. This means
# we always have a thread available to respond to an incoming cancel request.

# QueryCtrl interface
#
# Operations:
#
# void cancel();
# void destroy();

struct CancelRequest
{
}

struct CancelResponse
{
}

struct DestroyRequest
{
}

struct DestroyResponse
{
}
