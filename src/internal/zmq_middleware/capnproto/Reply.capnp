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

@0xfdb6a0374075229f;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto::Reply");

using ValueDict = import "ValueDict.capnp";

# Reply interface
#
# Callback interface for the results returned by a query. We instantiate a separate reply object
# for each query and pass a proxy to the reply object to createQuery(). In turn, the reply proxy
# ends up inside the scope implementation, which uses it to push the results. This means that the
# query identity is implicit in the reply object that is passed. The reply object is registered
# on its object adapter with a UUID as the identity.
#
# Operations:
#
# void push(string result);
# enum FinishedReason { Finished, Cancelled, Error };
# void finished(Reason r);

struct PushRequest
{
    result @0 : ValueDict.ValueDict;
}

enum FinishedReason
{
    unused @0;
    finished @1;
    cancelled @2;
    error @3;
}

struct FinishedRequest
{
    reason @0 : FinishedReason;
    error @1  : Text;               # Present only if reason is error
}
