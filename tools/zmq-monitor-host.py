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

# Binary used to host the monitoring proxy, which forwards messages
# from all monitor publishers to all subscribers.
# To use the zmq-parser.py you need to have this daemon running.

import zmq

context = zmq.Context()

aggregator = context.socket(zmq.XSUB)
aggregator.bind("ipc:///tmp/scopes-monitor")

sender = context.socket(zmq.XPUB)
sender.bind("ipc:///tmp/scopes-monitor2")

poller = zmq.Poller()
poller.register(aggregator, zmq.POLLIN)
poller.register(sender, zmq.POLLIN)

while True:
    sockets = dict(poller.poll())
    # forward XSUB messages to XPUB and vice versa
    if aggregator in sockets:
        msg = aggregator.recv()
        sender.send(msg)
    if sender in sockets:
        msg = sender.recv()
        aggregator.send(msg)
