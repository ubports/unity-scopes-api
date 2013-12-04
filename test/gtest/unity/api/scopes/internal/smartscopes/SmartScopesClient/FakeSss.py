#!/usr/bin/python
from wsgiref.simple_server import make_server

def response(environ, start_response):
	status = '200 OK'
	response_headers = [('Content-Type', 'application/json'), ('Content-Length', str(len(response_body)))]
    
	start_response(status, response_headers)
	return [response_body]
	
response_body = '[{"search_url": "https://productsearch.ubuntu.com/smartscopes/v2/search/demo", "name": "Dummy Demo Scope"}]'

httpd = make_server('', 9009, response)
httpd.handle_request()

del httpd

response_body = '{"category": {"renderer_template": "({\"template\":{\"renderer\":\"grid\",", "id": "cat1", "title": "Category 1"}}\
\
{"result": {"cat_id": "cat1", "art": "https://productsearch.ubuntu.com/imgs/amazon.png", "uri": "URI", "title": "Stuff"}}\
\
'

httpd = make_server('', 9009, response)
httpd.handle_request()
