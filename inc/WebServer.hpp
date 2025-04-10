#pragma once

#include "webserv.hpp"
#include "Clients.hpp"
#include <cerrno>

class Response;


class WebServer {
    private:
        std::string Host;
        int serverFd, addrLen, Port;
        std::vector<pollfd> pollFds;
        std::vector<Clients> clients;
        struct addrinfo first, *res;

    private:
        void ServerResponse(Clients &);
        void setNonBlocking(int fd);
        int SocketCreator(const std::string &host);
        struct sockaddr_in address;
        bool CheckResponse(Clients &client, std::string &headers);
        void addClient(int fd, short events);
        void closeClient(int index);
        void CGIHandle(Clients &client);
        void readFormData(int i); // get rest data
        int new_connection();


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
