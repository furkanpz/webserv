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

#ifdef __APPLE__
    #include <sys/event.h>
    #define POLLER() kqueue()
    #define EVENT_STRUCT struct kevent
#else
    #include <sys/epoll.h>
    #define POLLER() epoll_create(1024)
    #define EVENT_STRUCT struct epoll_event
#endif

#define MAX_EVENTS 64


class WebServer {
    private:
        std::string Host;
        int serverFd, pollFd, addrLen, Port;        
        struct addrinfo first, *res;

    private:
        void ServerResponse(int);
        void setNonBlocking(int fd);
        int SocketCreator(const std::string &host);
        struct sockaddr_in address;

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
