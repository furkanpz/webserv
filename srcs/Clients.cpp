#include "Clients.hpp"

Clients::Clients(int fd, unsigned long mbs, Server &server) : fd(fd), formData(""), Events(REQUEST),
    maxBodySize(mbs), server(server), writeBuffer(""), writeOffset(0)
{

}

Clients::~Clients()
{
}

void Clients::clearClient(void)
{
    this->response = Response();
    this->formData = "";
    this->Events = REQUEST;
    this->writeBuffer.clear();
    this->writeOffset = 0;
}

