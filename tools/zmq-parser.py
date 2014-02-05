#!/usr/bin/env python

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
# Authored by: Michal Hruby <michal.hruby@canonical.com>
#

# A monitor listening to all scopes capnproto messages passed via zeromq.
# Meant to be used for debugging. Note that you need to enable the monitoring
# code inside ZmqObject for this to capture anything - this is currently done
# via a cmake option (-DIPC_MONITORING=ON). You also need to run
# the zmq-monitor-host.py script.

import zmq
import sys
import capnp
from datetime import datetime

schema_base_dir = "../src/scopes/internal/zmq_middleware/capnproto/"
# location of c++.capnp
import_dirs = ["/usr/include", "/usr/local/include"]

# Careful here! Order matters, capnp can crash otherwise
proxy_capnp = capnp.load(schema_base_dir + "Proxy.capnp", None, import_dirs)
valdict_capnp = capnp.load(schema_base_dir + "ValueDict.capnp", None, import_dirs)
scope_capnp = capnp.load(schema_base_dir + "Scope.capnp", None, import_dirs)
scoped_capnp = capnp.load(schema_base_dir + "ScopeDict.capnp", None, import_dirs)
msg_capnp = capnp.load(schema_base_dir + "Message.capnp", None, import_dirs)
query_capnp = capnp.load(schema_base_dir + "Query.capnp", None, import_dirs)
queryc_capnp = capnp.load(schema_base_dir + "QueryCtrl.capnp", None, import_dirs)
reg_capnp = capnp.load(schema_base_dir + "Registry.capnp", None, import_dirs)
reply_capnp = capnp.load(schema_base_dir + "Reply.capnp", None, import_dirs)

context = zmq.Context()

receiver = context.socket(zmq.SUB)
# the second argument can't be unicode, using encode() will make it work in py3
receiver.setsockopt(zmq.SUBSCRIBE, "".encode())
receiver.connect("ipc:///tmp/scopes-monitor2")

def increase_indent(text, num_spaces = 2):
    prefix = " " * num_spaces
    return "\n".join(map(lambda x: prefix + x, text.split('\n')))

msg_num = 0

while True:
    message = receiver.recv(0, False)
    cur_time = datetime.now().strftime("%H:%M:%S.%f")
    print(cur_time, "received", len(message.bytes), "byte message:")
    # FIXME: how come we never see Reply messages?
    request = msg_capnp.Request.from_bytes(message.bytes)
    print(request)
    inParams = request.inParams
    nested = None
    if inParams:
        # capnp message don't include their own schema, so let's hackity hack
        if request.opName == "create_query":
            nested = inParams.as_struct(scope_capnp.CreateQueryRequest)
        elif request.opName == "run":
            nested = inParams.as_struct(query_capnp.RunRequest)
        elif request.opName == "push":
            nested = inParams.as_struct(reply_capnp.PushRequest)

        if nested:
            print("  inParams as <%s>:" % nested.schema.node.displayName)
            print(increase_indent(str(nested)))
        elif request._has("inParams"):
            print("  inParams: [unknown message type]")
    print("==========================================")

    msg_num = msg_num + 1
