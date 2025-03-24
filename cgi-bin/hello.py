#!/usr/bin/env python3

import os
import cgi, cgitb

print("Content-Type: text/html")
print()
print("<html><body>")
print("<h1>Merhaba, CGI!</h1>")
print("<p>REQUEST_METHOD: {}</p>".format(os.environ.get("REQUEST_METHOD", "N/A")))
print("<p>QUERY_STRING: {}</p>".format(os.environ.get("QUERY_STRING", "N/A")))
print("</body></html>")
