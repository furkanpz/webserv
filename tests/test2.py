import socket

def send_chunked_request(host, port):
    # Bağlantıyı başlat
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((host, port))

    # HTTP başlıkları
    headers = "POST /cgi-bin/displayForm.py HTTP/1.1\r\n"
    headers += "Host: " + host + "\r\n"
    headers += "Transfer-Encoding: chunked\r\n"
    headers += "Content-Type: application/x-www-form-urlencoded\r\n"
    headers += "Connection: close\r\n\r\n"

    # İlk chunk
    chunk1 = "name=test1&surname=test2"
    chunk1_size = hex(len(chunk1))[2:]  # Chunk boyutunu hex formatında al

    # İkinci chunk
    chunk2 = "&age=test3&nickname=test4"
    chunk2_size = hex(len(chunk2))[2:]

    # Son chunk (boyut 0 ile işaretlenir)
    chunk_end = "0\r\n\r\n"

    # Verilerin chunked formatında birleşik olarak hazırlanması
    full_request = headers
    full_request += (chunk1_size + "\r\n").encode().decode()  # Chunk1 boyutu
    full_request += chunk1  # Chunk1 verisi
    full_request += "\r\n"  # Chunk1 sonu

    full_request += (chunk2_size + "\r\n").encode().decode()  # Chunk2 boyutu
    full_request += chunk2  # Chunk2 verisi
    full_request += "\r\n"  # Chunk2 sonu

    full_request += chunk_end  # Son chunk

    # Veriyi tek seferde gönder
    client_socket.sendall(full_request.encode())

    # Sunucudan gelen yanıtı al
    response = client_socket.recv(1024)
    print("Response:", response.decode())

    # Bağlantıyı kapat
    client_socket.close()

# Sunucuya bağlanın ve chunked veri gönderin
send_chunked_request("127.0.0.1", 4444)
