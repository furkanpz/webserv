#pragma once

#include "WebServer.hpp"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <vector>

#define MAX_EVENTS 10 

class Response;

class WebServer {
    private:
        std::string Host;
        int serverFd, addrLen, Port;
        pollfd *pollFd;
        std::vector<pollfd> pollFds;
        struct addrinfo first, *res;

    private:
        void ServerResponse(int);
        void setNonBlocking(int fd);
        int SocketCreator(const std::string &host);
        struct sockaddr_in address;
        
        void CGIHandle(int clientFd, Response &res);

    public:
        void start();
    
    public:
        WebServer(const std::string &host, int port);
        ~WebServer();

        class ServerExcp : public std::exception {
            private:
                std::string excp;
            public:
				virtual const char *what() const throw();
                ServerExcp(const std::string &);
                ~ServerExcp() throw();
        };
};
