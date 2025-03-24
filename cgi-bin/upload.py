#!/usr/bin/env python3
import os
import cgi
import cgitb

cgitb.enable()

up_dir = "../upload"

print("Content-Type: text/html\n")
form = cgi.FieldStorage()

if "file" not in form:
    print("<h1>Error: No file provided</h1>")
    exit(1)

file_item = form["file"]


if file_item.filename:
    filename = os.path.basename(file_item.filename)
    save_path = os.path.join(up_dir, filename)

    with open(save_path, "wb") as f:
        f.write(file_item.file.read())

    print(f"<h1>Upload Successful</h1>")
    print(f"<p>File <b>{filename}</b> has been uploaded successfully!</p>")
else:
    print("<h1>Error: No file uploaded</h1>")