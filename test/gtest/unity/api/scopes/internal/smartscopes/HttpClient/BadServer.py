#!/usr/bin/python
from wsgiref.simple_server import make_server

def response(environ, start_response):
	status = '404 Not Found'
	response_headers = []
    
	start_response(status, response_headers)
	return ""
	
httpd = make_server('127.0.0.1', 9009, response)
httpd.handle_request()
