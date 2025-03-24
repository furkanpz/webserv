#!/usr/bin/python3

import cgi
import os

# Dosya yükleme dizini
UPLOAD_DIR = "~/upload_files"

# Form verilerini al
form = cgi.FieldStorage()

# Dosya varsa işle
if "file" in form:
    file_item = form["file"]

    if file_item.filename:
        filepath = os.path.join(UPLOAD_DIR, file_item.filename)

        # Dosyayı kaydet
        with open(filepath, "wb") as f:
            f.write(file_item.file.read())
        
        print("Content-type: text/html\n")
        print("<html><body><h2>Dosya başarıyla yüklendi!</h2></body></html>")
    else:
        print("Content-type: text/html\n")
        print("<html><body><h2>Dosya yüklenemedi.</h2></body></html>")

else:
    print("Content-type: text/html\n")
    print("<html><body><h2>Formda dosya bulunamadı.</h2></body></html>")
