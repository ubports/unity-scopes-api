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

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto");

# ValueDict definition. A dictionary of name/Value pairs, where each Value can hold an int, bool, string or dict.

struct Value
{
    union
    {
        intVal    @0 : Int32;
        boolVal   @1 : Bool;
        stringVal @2 : Text;
        dictVal   @3 : ValueDict;
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
