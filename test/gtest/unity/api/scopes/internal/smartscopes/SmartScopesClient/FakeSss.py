#!/usr/bin/python
from wsgiref.simple_server import make_server

def response(environ, start_response):
    status = '200 OK'
    response_headers = [('Content-Type', 'application/json')]
    start_response(status, response_headers)

    if environ['PATH_INFO'] == '/smartscopes/v2/remote-scopes':
        return [remote_scopes_response]

    if environ['PATH_INFO'] == '/smartscopes/v2/search/demo' and environ['QUERY_STRING'] != '':
        return [search_response]

    return ''

remote_scopes_response = '\
[{"search_url": "https://productsearch.ubuntu.com/smartscopes/v2/search/demo", "name": "Dummy Demo Scope" },\
{"search_url": "https://productsearch.ubuntu.com/smartscopes/v2/search/demo2", "name": "Dummy Demo Scope 2", "invisible": true }]'

search_response = '\
{"category": {"renderer_template": "", "id": "cat1", "title": "Category 1"}}\r\n\
{"result": {"cat_id": "cat1", "art": "https://productsearch.ubuntu.com/imgs/amazon.png", "uri": "URI", "title": "Stuff"}}\r\n\
{"result": {"cat_id": "cat1", "art": "https://productsearch.ubuntu.com/imgs/google.png", "uri": "URI2", "title": "Things"}}'

httpd = make_server('', 9009, response)
httpd.handle_request()
