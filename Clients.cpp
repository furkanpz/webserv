#include "Clients.hpp"

Clients::Clients(pollfd &newPoll, int fd, int index, unsigned long mbs, Server &server) : requestType(-1), poll(&newPoll), fd(fd),
    formData(""), events(REQUEST), index(index), maxBodySize(mbs), server(server), CGI_DONE(true), CGI_pid(0), CGI_fd_out(0), CGI_fd_in(0)
{

}

Clients::~Clients()
{
}

int Clients::getRequestType() const{
    return requestType;
}

void Clients::setRequestType(int _requestType){
    this->requestType = _requestType;
}

void Clients::client_send(int _int, const void *v, size_t s)
{
    send(_int, v, s, 0);
    this->response = Response();
    this->formData = "";
    this->events = REQUEST;
}