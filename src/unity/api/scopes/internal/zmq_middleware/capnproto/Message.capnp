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

@0x82174b92ecb836f7;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto");

# Message definitions (request and response). 

enum RequestMode
{
    oneway @0;
    twoway @1;
}

# A request contains the invocation mode, identity of the target object, an operation name, and the
# in-parameters as a blob.

struct Request
{
    mode     @0 : RequestMode;   # Response required?
    id       @1 : Text;          # Identity of target object
    opName   @2 : Text;          # Operation name
    inParams @3 : Object;        # In-parameters for the operation
}

# Responses indicate success or an exception. All twoway invocations can raise run-time exceptions (such
# as OperationNotExistException or ObjectNotExistException).
# Operation can optionally raise one of a number of user-defined exceptions.

enum ResponseStatus
{
    success          @0;
    runtimeException @1;
    userException    @2;
}

# A response is the response status, plus any payload. The payload is the out-parameters and return value (if any),
# or it contains the details of the exception indicated by the status.

struct Response
{
    status  @0 : ResponseStatus;
    payload @1 : Object;            # Out-params followed by return value (if any), or exception data
}

# Run-time exceptions

using Proxy = import "Proxy.capnp";

struct OperationNotExistException
{
    proxy  @0 : Proxy.Proxy;        # The proxy for the request that raised the exception
    opName @1 : Text;               # Name of operation that wasn't found
}

struct ObjectNotExistException
{
    proxy  @0 : Proxy.Proxy;        # The proxy for the request that raised the exception
}

# Generic representation of a runtime exception. Holds the type of the exception plus a union of all the exceptions
# that are possible.

enum RuntimeExceptionType
{
    operationNotExist @0;
    objectNotExist    @1;
}

struct RuntimeException
{
    type @0 : RuntimeExceptionType;
    exception : union 
    {
        operationNotExist @1 : OperationNotExistException;
        objectNotExist    @2 : ObjectNotExistException;
    }
}
