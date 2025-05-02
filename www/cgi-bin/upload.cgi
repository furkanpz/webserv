#!/usr/bin/python3

import cgi
import os
import signal

signal.signal(signal.SIGPIPE, signal.SIG_DFL)

UPLOAD_DIR = "../../uploads"

if not os.path.exists(UPLOAD_DIR):
    os.makedirs(UPLOAD_DIR)

form = cgi.FieldStorage()



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
        <h2>Dosya Yükle</h2>
"""


if "file" in form.keys():
    file_item = form["file"]
    
    if file_item.filename:
        file_path = os.path.join(UPLOAD_DIR, file_item.filename)
        
        with open(file_path, "wb") as f:
            f.write(file_item.file.read())
        
        response += f"""
        <div class="message success">
            <p><strong>Success!</strong> File <em>{file_item.filename}</em> Dosya Başarıyla Yüklendi!</p>
        </div>
        """
    else:
        response += """
        <div class="message error">
            <p><strong>Error!</strong> Dosya Yüklenmedi!.</p>
        </div>
        """
else:
    response += """
    <div class="message error">
        <p><strong>Error!</strong> Dosya Yok!</p>
    </div>
    """

response += """
    <a href="/" class="button">Anasayfa</a>
    </div>
</body>
</html>
"""
print(response)
