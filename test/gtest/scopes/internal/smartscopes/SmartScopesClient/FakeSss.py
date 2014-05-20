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

preview1_complete = False

def response(environ, start_response):
    global preview1_complete
    status = '200 OK'
    response_headers = [('Content-Type', 'application/json')]
    start_response(status, response_headers)

    if environ['PATH_INFO'] == '/remote-scopes' and (environ['QUERY_STRING'] == '' or environ['QUERY_STRING'] == 'locale=test_TEST'):
        return [remote_scopes_response]

    if environ['PATH_INFO'] == '/demo/search' and environ['QUERY_STRING'] != '':
        return [search_response]

    if environ['PATH_INFO'] == '/demo/preview' and environ['QUERY_STRING'] != '':
        if preview1_complete == False:
            preview1_complete = True
            return [preview_response]
        if preview1_complete == True:
            return [preview_response2]

    return ''

serving = False
port = 1024
while serving == False:
    try:
        httpd = make_server('127.0.0.1', port, response)
        serving = True
    except:
        port += 1

print(str(port))
sys.stdout.flush()

remote_scopes_response = '\
[{"base_url": "http://127.0.0.1:' + str(port) + '/fail", "id" : "fail.scope", "name": "Fail Scope", "description": "Fails due to no author.", "icon": "icon" },\
{"base_url": "http://127.0.0.1:' + str(port) + '/demo", "id" : "dummy.scope", "name": "Dummy Demo Scope", "description": "Dummy demo scope.", "author": "Mr.Fake", "icon": "icon" },\
{"base_url": "http://127.0.0.1:' + str(port) + '/fail2", "name": "Fail Scope 2", "description": "Fails due to no id.", "author": "Mr.Fake", "icon": "icon" },\
{"base_url": "http://127.0.0.1:' + str(port) + '/demo2", "id" : "dummy.scope.2", "name": "Dummy Demo Scope 2", "description": "Dummy demo scope 2.", "author": "Mr.Fake", "art": "art", "invisible": true, "appearance": {"background": "#00BEEF"} },\
{"id" : "fail.scope.3", "name": "Fail Scope 3", "description": "Fails due to no base_url.", "author": "Mr.Fake", "art": "art" }]'

search_response = '\
{"category": {"render_template": "{}", "id": "cat1", "title": "Category 1"}}\r\n\
{"result": {"cat_id": "cat1", "art": "https://dash.ubuntu.com/imgs/amazon.png", "uri": "URI", "title": "Stuff"}}\r\n\
{"result": {"cat_id": "cat1", "icon": "https://dash.ubuntu.com/imgs/google.png", "uri": "URI2", "title": "Things"}}\r\n\
{"result": {"cat_id": "cat2", "art": "https://dash.ubuntu.com/imgs/cat_fail.png", "uri": "URI3", "title": "Category Fail"}}'

preview_response = '\
{"columns": [[["widget_id_A", "widget_id_B", "widget_id_C"]], [["widget_id_A"], ["widget_id_B", "widget_id_C"]], [["widget_id_A"], ["widget_id_B"], ["widget_id_C"]]]}\r\n\
{"widget": {"id": "widget_id_A", "type": "text", "title": "Widget A", "text": "First widget."}}\r\n\
{"widget": {"id": "widget_id_B", "type": "text", "title": "Widget B", "text": "Second widget."}}\r\n\
{"widget": {"id": "widget_id_C", "type": "text", "title": "Widget C", "text": "Third widget."}}'

preview_response2 = '\
{"widget": {"id": "widget_id_A", "type": "text", "title": "Widget A", "text": "First widget."}}\r\n\
{"widget": {"id": "widget_id_B", "type": "text", "title": "Widget B", "text": "Second widget."}}'

httpd.serve_forever()
