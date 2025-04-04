#pragma once

#include "webserv.hpp"
#include "Response.hpp"

class Clients {
    private:
        int         requestType;
        
    public:
        int         events;
        std::string formData;
        pollfd      *poll;
        int         fd;
        Response response;

        Clients(pollfd &newPoll, int);
        ~Clients();
        void client_send(int _int, const void *v, size_t s);
        int         getFd() const;
        void        setFd(int _fd);
        int         getRequestType() const;
        void        setRequestType(int _requestType);
        
};