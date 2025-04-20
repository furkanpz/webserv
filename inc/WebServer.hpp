#pragma once

#include "webserv.hpp"
#include "Clients.hpp"
#include <cerrno>

class Response;


class WebServer {
    private:
        std::vector<Server> &w_servers;
        std::vector<pollfd> pollFds;
        std::vector<Clients> clients;
        int serverSize;
        Server server;
    private:
        void ServerResponse(Clients &);
        void setNonBlocking(int fd);
        int SocketCreator(Server &server);
        bool CheckResponse(Clients &client, std::string &headers);
        void addClient(int fd, short events, size_t i);
        void closeClient(int index);
        void CGIHandle(Clients &client);
        void readFormData(int i);
        int new_connection(size_t i);
        void CGIEXECUTE(Clients &client, int fd_out[2], int fd_in[2]);
        void ServersCreator(std::vector<Server> &servers);

    public:
        void start();
    
    public:
        WebServer(std::vector<Server> &servers);
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
