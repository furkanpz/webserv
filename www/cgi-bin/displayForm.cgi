#!/usr/bin/env python3
import cgi

form = cgi.FieldStorage()
try :
    name = form.getvalue("name", None)
    surname = form.getvalue("surname", None)
    age = form.getvalue("age", None)
    nickname = form.getvalue("nickname", None)
except:
    header = "HTTP/1.1 400 Bad Request\r\n"
    response = "Invalid input"
    print(header + response)
    exit()

response = f"""<!DOCTYPE html>
<html lang="tr">
<head>
    <meta charset="UTF-8">
    <title>Giriş Bilgileri</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }}
        .container {{
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            text-align: center;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h2>42 Profile</h2>
        <p>Name: {name}</p>
        <p>Surname: {surname}</p>
        <p>Age: {age}</p>
        <p>Nickname: {nickname}</p>
        <a href="/index.html">Geri Dön</a>
    </div>
</body>
</html>"""

if (name is None or surname is None or age is None or nickname is None):
    header = "HTTP/1.1 400 Bad Request\r\n"
else:
    header = "HTTP/1.1 200 OK\r\n"

header += "Content-Type: text/html\r\nContent-Length: " + str(len(response)) + "\r\n\r\n"
print(header + response)