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

# Form verilerini ekrana yazdır
for field in form.keys():
    field_item = form[field]
    if field_item.filename:  # Dosya yüklenmişse
        print(f"File field: {field_item.filename} <br>")
        # Dosya yükleme işlemi yapılabilir
    else:
        print(f"Field name: {field}, Value: {field_item.value} <br>")

print("</body></html>")
