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

@0xfdb6a03748b7779f;

using Cxx = import "/capnp/c++.capnp";

$Cxx.namespace("unity::api::scopes::internal::zmq_middleware::capnproto::Object");

# Object interface
#
# Operations:
#
# void ping();

# ping() has no parameters, so there is neither a Request nor a Response struct.
