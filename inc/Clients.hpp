#pragma once

#include "webserv.hpp"
#include "Response.hpp"

class Clients {
    public:
        int         fd;
        std::string formData;
        int         Events;
        Response    response;
        unsigned long maxBodySize;
        Server server;
        Servers servers;
        std::string writeBuffer;
        size_t      writeOffset;
        std::string ServerName;

        Clients(int, Server &, Servers &);
        ~Clients();
        void clearClient(void);
        
};