#pragma once

#include "webserv.hpp"

class WebServer {
    public:
         WebServer(const std::string &host, int port);
        ~WebServer();
        std::string Host;
        int serverFd, socketServ, addrLen;
        int Port;
        struct sockaddr_in address;

        void start();
};

