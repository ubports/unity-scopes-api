#!/usr/bin/python
from wsgiref.simple_server import make_server
import time
import sys

def response(environ, start_response):
        time.sleep(int(sys.argv[1]))
        status = '200 OK'
        response_headers = [('Content-Type', 'text/plain'), ('Content-Length', str(len(response_body)))]

        start_response(status, response_headers)
        return [response_body]

response_body = 'Hello there'

httpd = make_server('127.0.0.1', 9009, response)
httpd.handle_request()
