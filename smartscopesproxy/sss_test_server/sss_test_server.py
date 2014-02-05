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
import sys

def response(environ, start_response):
    status = '200 OK'
    response_headers = [('Content-Type', 'application/json')]
    start_response(status, response_headers)

    if environ['PATH_INFO'] == '/remote-scopes':
        return [remote_scopes_response]

    return ''

remote_scopes_response = '\
[{"base_url": "https://productsearch.ubuntu.com/demo", "name": "DummyScope", "description": "Dummy scope." },\
{"base_url": "https://productsearch.ubuntu.com/demo2", "name": "DummyScope2", "description": "Dummy scope 2.", "invisible": false },\
{"base_url": "https://productsearch.ubuntu.com/demo3", "name": "DummyScope3", "description": "Dummy scope 3.", "invisible": true },\
{"base_url": "https://productsearch.ubuntu.com/demo4", "name": "DummyScope4", "description": "Dummy scope 4." },\
{"base_url": "https://productsearch.ubuntu.com/demo5", "name": "DummyScope5", "description": "Dummy scope 5.", "invisible": true }]'

httpd = make_server('127.0.0.1', 8000, response)
print('Serving on http://127.0.0.1:8000')
httpd.serve_forever()
