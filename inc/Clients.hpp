#pragma once

#include "webserv.hpp"
#include "Response.hpp"

class Clients {
    public:
        int         fd;
        std::string formData;
        int         events;
        Response    response;
        unsigned long maxBodySize;
        Server server;
        std::string writeBuffer;
        size_t      writeOffset;

        Clients(int, unsigned long, Server &);
        ~Clients();
        void clearClient(void);
        int         getFd() const;
        void        setFd(int _fd);
        int         getRequestType() const;
        void        setRequestType(int);
        
};