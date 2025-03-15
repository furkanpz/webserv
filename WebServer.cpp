#include "WebServer.hpp"

    
WebServer::WebServer(const std::string &host, int port) : Host(host), Port(port) {
    addrLen = sizeof(address);
    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(Port);
    
    if (host == "localhost") {
        address.sin_addr.s_addr = INADDR_ANY;
    } else {
        address.sin_addr.s_addr = inet_addr(host.c_str());
    }
    if (serverFd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
}

WebServer::~WebServer() {
    close(serverFd);
}

void WebServer::start() {
    if (listen(serverFd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on " << Host << ":" << Port << "...\n";

    while (true) {
        socketServ = accept(serverFd, (struct sockaddr *)&address, (socklen_t*)&addrLen);
        if (socketServ < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        char buffer[30000] = {0};
        read(socketServ, buffer, 30000);
        std::cout << "Received request:\n" << buffer << std::endl;
        
        const char *response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 13\r\n\r\nHello, World!";
        send(socketServ, response, strlen(response), 0);
        close(socketServ);
    }
}