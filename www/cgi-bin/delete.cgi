#!/usr/bin/python3

import cgi
import os

UPLOAD_DIR = "../../uploads"


form = cgi.FieldStorage()

try :
    filename = form.getvalue("filename")
except:
    header = "HTTP/1.1 400 Bad Request\r\n"
    response = "Invalid input"
    print(header + response)
    exit()

flag = 0
response = """
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>File Upload</title>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            background-color: #f4f4f9;
            color: #333;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }
        .container {
            background-color: #fff;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            padding: 20px;
            width: 80%;
            max-width: 600px;
        }
        h2 {
            text-align: center;
            color: #4CAF50;
        }
        .message {
            padding: 15px;
            margin: 10px 0;
            border-radius: 5px;
        }
        .success {
            background-color: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .error {
            background-color: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .upload-info {
            margin-top: 20px;
            font-size: 16px;
        }
        .button {
            display: block;
            margin: 20px auto;
            padding: 10px 20px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 16px;
        }
        .button:hover {
            background-color: #45a049;
        }
    </style>
</head>
<body>
    <div class="container">
        <h2>Dosya Sil</h2>
"""


if filename:
    path = os.path.join(UPLOAD_DIR, filename)
    try:
        os.remove(path)
        response += f"""
        <div class="message success">
            <p><strong>Success! </strong><em>{filename}</em> başarıyla silindi.</p>
        </div>
        """
        flag = 1
    except Exception as e:
        response += f"""
        <div class="message error">
            <p><strong>Error!</strong> Dosya silinemedi</p>
        </div>
        """
else:
    response += """
    <div class="message error">
    <p><strong>Error!</strong> Geçersiz dosya adı.</p>
    </div>
    """

response += """
    <a href="/" class="button">Anasayfa</a>
    </div>
</body>
</html>
"""

print(response)
