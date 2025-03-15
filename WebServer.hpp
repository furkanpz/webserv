#pragma once

#include "webserv.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

class WebServer {
    public:
        int serverFd, socketServ, addrLen;
        int Port;
        std::string Host;
        struct sockaddr_in address;

        WebServer(const std::string &host, int port);
        ~WebServer();
        void start();
};

