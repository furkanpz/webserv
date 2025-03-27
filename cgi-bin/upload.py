#!/usr/bin/python3

import cgi
import os

# Dosya yükleme dizini
UPLOAD_DIR = "../upload_files"

# Form verilerini al
form = cgi.FieldStorage()

# Form verilerinin çıktısını yazdır (debug amaçlı)
print("Content-Type: text/html")
print()  # HTTP başlıkları arasındaki boş satır

print("<html><body>")
print("<h2>Form Verisi:</h2>")

print(form)

print("</body></html>")
