#include "Clients.hpp"

Clients::Clients(int fd, Server &server, Servers &servers) : fd(fd), formData(""), Events(REQUEST), server(server), servers(servers), writeBuffer(""), writeOffset(0)
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
