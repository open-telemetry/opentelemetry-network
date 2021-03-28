#!/usr/bin/env python3

import http.server

class RequestHandler (http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        payload = b'{"token":"XYZ","issuedAtS":"1590626143","expirationS":"1590626743"}'

        self.send_response(200)
        self.send_header("Content-Length", len(payload))
        self.end_headers()
        self.wfile.write(payload)


server_address = ('', 8000)
httpd = http.server.HTTPServer(server_address, RequestHandler)
httpd.serve_forever()
