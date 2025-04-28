#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <random>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 3131
#define CLIENT_COUNT 5  // Aynı anda 100 istemci
#define REQUESTS_PER_CLIENT 5  // Her istemci 5 istek göndersin

std::vector<std::string> methods = {"GET", "POST", "DELETE"};
std::vector<std::string> paths = {"/"};

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> sleepTime(100, 500);  // Rastgele bekleme (100-500 ms)
std::uniform_int_distribution<> methodDist(0, methods.size() - 1);
std::uniform_int_distribution<> pathDist(0, paths.size() - 1);

void sendRequest(int clientID, const std::string& method, const std::string& path, const std::string& body = "") {
    int sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sock == -1) {
        std::cerr << "[İstemci " << clientID << "] Socket oluşturulamadı!\n";
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[İstemci " << clientID << "] Bağlantı hatası!\n";
        close(sock);
        return;
    }

    std::string request = method + " " + path + " HTTP/1.1\r\n";
    request += "Host: " + std::string(SERVER_IP) + "\r\n";
    request += "Connection: keep-alive\r\n";

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
    
    for (int i = 0; i < REQUESTS_PER_CLIENT; i++) {
        std::string method = methods[methodDist(gen)];
        std::string path = paths[pathDist(gen)];
        std::string body = (method == "POST") ? "İstemci " + std::to_string(clientID) + " veri gönderdi." : "";

        sendRequest(clientID, method, path, body);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime(gen))); // Rastgele bekleme süresi
    }
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
