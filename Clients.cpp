#include "Clients.hpp"

Clients::Clients(pollfd &newPoll, int fd) : requestType(-1), poll(&newPoll), fd(fd),
    formData(""), events(REQUEST)
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
    this->response.setRequestType(NONE);
    this->response.setResponseCode(NONE);
    this->response.setContentTypeForPost("");
    this->response.setContent("");
    this->response.setFile("");
    this->response.setContentLength(NONE);
    this->response.setcontentType("");
    this->response.setisCGI(false);
    this->formData = "";
    this->events = REQUEST;
}