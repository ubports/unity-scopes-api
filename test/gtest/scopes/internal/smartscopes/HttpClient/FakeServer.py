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
# Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
#

from wsgiref.simple_server import make_server
from random import randint
import sys
import time

def response(environ, start_response):
    if environ['QUERY_STRING'] == 'x':
        status = '404 Not Found'
        response_headers = [('Content-Type', 'text/plain')]
        start_response(status, response_headers)
        return ''
    else:
        status = '200 OK'
        response_headers = [('Content-Type', 'text/plain')]
        if environ['QUERY_STRING'] != '':
            time.sleep(int(environ['QUERY_STRING']))
        start_response(status, response_headers)
        return 'Hello there'

serving = False
port = randint(49152, 65535)
while serving == False:
    try:
        httpd = make_server('127.0.0.1', port, response)
        serving = True
    except:
        port = randint(49152, 65535)

print(str(port))
sys.stdout.flush()

httpd.serve_forever()
