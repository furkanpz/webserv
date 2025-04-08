import socket
import os

def send_chunked_request_with_pdf(host, port, file_path):
    # Bağlantıyı başlat
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))

    # HTTP başlıkları
    headers = "POST /cgi-bin/upload.py HTTP/1.1\r\n"
    headers += "Host: " + host + "\r\n"
    headers += "Transfer-Encoding: chunked\r\n"
    headers += "Content-Type: application/pdf\r\n"
    headers += "Connection: close\r\n\r\n"

    # PDF dosyasını oku
    with open(file_path, 'rb') as f:
        file_data = f.read()

    # PDF verisini chunked formata bölelim
    chunk_size = 1024  # her chunk'ın büyüklüğü 1 KB
    full_request = headers.encode()  # headers'ı baştan byte formatında başlatıyoruz
    index = 0
    i = 0
    while index < len(file_data):
        # Her chunk'ın boyutunu hexadecimal formatta al
        chunk = file_data[index:index + chunk_size]
        chunk_length = hex(len(chunk))[2:]
        # Chunk'ı ekle
        full_request += (chunk_length + "\r\n").encode()  # Chunk boyutu byte formatına çevir
        full_request += chunk  # Chunk verisini byte formatında ekle
        full_request += b"\r\n"  # Chunk sonu (byte formatında)
        index += chunk_size

    full_request += b"0\r\n\r\n"

    # Veriyi tek seferde gönder
    client_socket.sendall(full_request)

    # Sunucudan gelen yanıtı al
    response = client_socket.recv(1024)
    print("Response:", response.decode())

    # Bağlantıyı kapat
    client_socket.close()

# Sunucuya bağlanın ve PDF dosyasını chunked olarak gönderin
send_chunked_request_with_pdf("127.0.0.1", 4444, "en.subject.pdf")
