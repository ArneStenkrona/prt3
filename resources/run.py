#!/usr/env python3
import http.server
import os
import logging

try:
    import http.server as server
except ImportError:
    # Handle Python 2.x
    import SimpleHTTPServer as server

class HTTPRequestHandler(server.SimpleHTTPRequestHandler):
    """
    SimpleHTTPServer with added bonus of:

    - handle PUT requests
    - log headers in GET request
    """

    def end_headers(self):
        self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
        self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
        server.SimpleHTTPRequestHandler.end_headers(self)

    def do_GET(self):
        server.SimpleHTTPRequestHandler.do_GET(self)
        logging.warning(self.headers)

    def do_PUT(self):
        """Save a file following a HTTP PUT request"""
        filename = os.path.join(os.getcwd(),self.translate_path(self.path))
        alt = os.getcwd() + self.path
        print("CWD = " + os.getcwd())
        print("file = " + filename)

        file_length = int(self.headers['Content-Length'])
        with open(filename, 'wb') as output_file:
            output_file.write(self.rfile    .read(file_length))
        self.send_response(201, 'Created')
        self.end_headers()
        reply_body = 'Saved "%s"\n' % filename
        self.wfile.write(reply_body.encode('utf-8'))

        ## Update data.js
        os.system("$EMSDK/upstream/emscripten/tools/file_packager prt3.data --preload assets/ --use-preload-plugins --js-output=data.js")


if __name__ == '__main__':
    server.test(HandlerClass=HTTPRequestHandler)
