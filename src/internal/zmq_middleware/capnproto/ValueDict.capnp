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

@0x9514134742fa960f;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::scopes::internal::zmq_middleware::capnproto");

struct NullVal
{
}

# ValueDict definition. A dictionary of name/Value pairs, where each 
# Value can hold an int, bool, string, double, dict, array or null.

struct Value
{
    union
    {
        intVal    @0 : Int32;
        doubleVal @1 : Float64;
        boolVal   @2 : Bool;
        stringVal @3 : Text;
        dictVal   @4 : ValueDict;
        arrayVal  @5 : List(Value);
        nullVal   @6 : NullVal;
    }
}

struct NVPair
{
    name  @0 : Text;
    value @1 : Value;
}

struct ValueDict
{
    pairs @0 : List(NVPair);
}
