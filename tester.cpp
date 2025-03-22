#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <thread>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 2323
#define CLIENT_COUNT 20  

void sendRequest(int clientID, const std::string& method, const std::string& path, const std::string& body = "") {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "[İstemci " << clientID << "] Socket oluşturulamadı\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[İstemci " << clientID << "] Bağlantı hatası\n";
        close(sock);
        return;
    }

    std::string request = method + " " + path + " HTTP/1.1\r\n";
    request += "Host: " + std::string(SERVER_IP) + "\r\n";
    request += "Connection: close\r\n";

    if (!body.empty()) {
        request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        request += "Content-Type: text/plain\r\n";
    }

    request += "\r\n" + body;

    send(sock, request.c_str(), request.size(), 0);

    char buffer[1024];
    int bytesReceived = read(sock, buffer, sizeof(buffer) - 1);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "[İstemci " << clientID << "] Yanıt:\n" << buffer << std::endl;
    }

    close(sock);
}

void runClient(int clientID) {
    std::cout << "[İstemci " << clientID << "] Sunucuya bağlanıyor...\n";
    sendRequest(clientID, "GET", "/");
    sendRequest(clientID, "POST", "/", "İstemci " + std::to_string(clientID) + " test verisi");
    sendRequest(clientID, "DELETE", "/resource");
}

int main() {
    std::vector<std::thread> clients;

    for (int i = 0; i < CLIENT_COUNT; ++i) {
        clients.emplace_back(runClient, i + 1);
    }

    for (auto& client : clients) {
        client.join();
    }

    std::cout << "Tüm istemciler işlemi tamamladı.\n";
    return 0;
}
