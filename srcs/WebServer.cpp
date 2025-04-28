#include "WebServer.hpp"
#include "Utils.hpp"
#include "Response.hpp"
#include "sys/wait.h"


int WebServer::SocketCreator(Server &server){
    
    server.addrLen = sizeof(server.address);
    server.address.sin_family = AF_INET;
    server.address.sin_port = htons(server.port);

    std::memset(&server.first, 0, sizeof(server.first));

    server.first.ai_family = AF_INET;
    server.first.ai_socktype = SOCK_STREAM;
    server.first.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(server.host.c_str(), Utils::intToString(server.port).c_str(), &server.first, &server.res);
    if (status != 0) {freeaddrinfo(server.res); throw ServerExcp("getaddrinfo Error");}
    return (socket(server.res->ai_family, server.res->ai_socktype, server.res->ai_protocol));
}

int WebServer::findPollIndex(int fd) {
    for (size_t j = 0; j < pollFds.size(); ++j) {
        if (pollFds[j].fd == fd)
            return j;
    }
    return -1;
}

void WebServer::CGIHandle(Clients &client)
{
    extern char **environ;
    int fd_out[2]; 
    int fd_in[2];
    if (pipe(fd_out) == -1 || pipe(fd_in) == -1)
        return;
    int pid = fork();
    if (pid < 0) { 
        close(fd_out[0]); close(fd_out[1]);
        close(fd_in[0]); close(fd_in[1]);
        return;
    }
    else if (pid == 0)
    {
        if (client.response.getRequestType() == POST || client.response.getRequestType() == DELETE) {
            setenv("CONTENT_TYPE", client.response.getcontentType().c_str(), 1);
            setenv("CONTENT_LENGTH", Utils::intToString(client.response.getContentLength()).c_str(), 1);
        }
        setenv("REQUEST_METHOD", methods[MAX_INT - client.response.getRequestType()].c_str(), 1);
        close(fd_out[0]);  
        dup2(fd_out[1], 1); 
        close(fd_out[1]);


        close(fd_in[1]); 
        dup2(fd_in[0], 0); 
        close(fd_in[0]);

        std::string file = client.response.getFile();
        std::string cgiPath = client.response.getCgiPath();
        const char *argv[] = { cgiPath.c_str() ,file.c_str(), NULL};
        execve(cgiPath.c_str(), const_cast<char *const *>(argv), environ);
        std::cerr << "ERROR " << std::endl;
        exit(1);
    }
    else {
        char        buffer[55555];
        std::string response;
        int         bytesRead;

        close(fd_out[1]); 
        close(fd_in[0]);

        if (client.response.getRequestType() == POST || client.response.getRequestType() == DELETE) 
        {    
            write(fd_in[1], client.response.getFormData().c_str(), client.response.getFormData().size());
            if (client.response.getFormData().empty())
                client.response.setResponseCode(BADREQUEST);
        }
        close(fd_in[1]); 
        bool finished = Utils::wait_with_timeout(pid, 2);
        if (finished)
        {
            while ((bytesRead = read(fd_out[0], buffer, sizeof(buffer))) > 0)
                response.append(buffer, bytesRead);
            close(fd_out[0]);
            client.response.setContent(response);
            client.writeBuffer = Utils::returnResponseHeader(client);
        }
        else
        {
            client.response.setResponseCode(TIMEOUT);
            client.response.setContent(Utils::returnErrorPages(client.response, TIMEOUT, client));
            client.writeBuffer = Utils::returnResponseHeader(client);
        }
    }
}

void WebServer::ServersCreator(std::vector<Server> &servers)
{
    for (size_t i = 0; i < servers.size(); i++)
    {
        Server &server = servers[i];
        server.serverFd = SocketCreator(servers[i]);
        if (server.serverFd == 0) {freeaddrinfo(server.res); close(server.serverFd); throw ServerExcp("Socket Error");}
        int opt = 1;
        if (setsockopt(server.serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(server.serverFd);
            freeaddrinfo(server.res);
            throw ServerExcp("Setsockopt Error");
        }
        std::cout << server.host << ":" << server.port << std::endl;
        if (bind(server.serverFd, server.res->ai_addr, server.res->ai_addrlen) < 0) {freeaddrinfo(server.res); close(server.serverFd); throw ServerExcp("Bind Error");}
        freeaddrinfo(server.res);
        if (listen(server.serverFd, SOMAXCONN) < 0) {close(server.serverFd); throw ServerExcp("Listen Error");}

        setNonBlocking(server.serverFd);
        addClient(server.serverFd, POLLIN, i);
        std::cout << "Server listening on " << server.host << ":" << server.port << "..." << std::endl;
    }
    serverSize = pollFds.size();
    std::cout << "Server size: " << serverSize << std::endl;
}
WebServer::WebServer(std::vector<Server> &servers) : w_servers(servers) {
    ServersCreator(servers);
}


WebServer::~WebServer()
{
    for (size_t x = 0; x < pollFds.size(); x++)
    {
        close(pollFds[x].fd);
    }
}

void WebServer::setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_RDWR | O_APPEND | O_NONBLOCK) == -1)
        throw ServerExcp("Fcntl Error");
}

bool WebServer::CheckResponse(Clients &client, std::string &headers)
{
    if (headers.find("Expect: 100-continue") != std::string::npos) {
        std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
        send(client.fd, continueResponse.c_str(), continueResponse.size(), 0);
    }
    Utils::parseContent(headers, client);
    if (client.response.getisCGI())
    {
        if (client.events == WAIT_FORM)
            return true;
        else
            return CGIHandle(client), true;
    }
    return false;
}


void printError(int fd) // DEBUG
{
    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) == 0) {
    if (err != 0) {
        // burada err errno gibi bir hata kodudur
        fprintf(stderr, "DEBUG - Socket error: %s\n", strerror(err));
    }
}
}

void WebServer::ServerResponse(Clients &client)
{
    std::string headers;
    char        buffer[10240] = {0};
    int         bytesRead;
    int         contentLength = 0;

    contentLength = 0;
    if (client.events == REQUEST)
    {
        while (true) {
            bytesRead = recv(client.fd, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead > 0)
            {
                headers.append(buffer, bytesRead);
                if (headers.find("\r\n\r\n") != std::string::npos)
                    break;
            }
            else 
            {
               if (bytesRead == -1)
            {
                std::cout << "RECV ERROR " << strerror(errno) << std::endl; // DEBUG
                return;
            }
                break;
            }

        }
    }
    if (CheckResponse(client, headers))
        return;
    client.writeBuffer = Utils::returnResponseHeader(client);;
}


void WebServer::addClient(int fd, short events, size_t i)
{
    pollfd clientPollFd = {fd, events, 0};
    Clients newClient(fd, this->w_servers[i].client_max_body_size, w_servers[i]);
    pollFds.push_back(clientPollFd);
    clients.push_back(newClient);
}

void WebServer::closeClient(int index) {
    int fd = pollFds[index].fd;
    if (fd < 0) 
        return;
    close(fd);
    pollFds[index].fd = -1;
    std::cout << "Client disconnected" << std::endl;
    clients.erase(clients.begin() + index);
    pollFds.erase(pollFds.begin() + index);
}

void WebServer::readFormData(int i)
{
    bool tempChunk = clients[i].response.getIsChunked();
    while (true)
    {
        char buffer[10240];
        int bytesRead = recv(clients[i].fd, buffer, sizeof(buffer), 0);
        if (bytesRead > 0) {
            clients[i].formData.append(buffer, bytesRead);
            if (tempChunk && clients[i].formData.find("0\r\n\r\n") != std::string::npos)
                Utils::parseChunked(clients[i], clients[i].formData, 1); 
        }
        else
        {
            if (bytesRead == -1)
            {
                std::cout << "RECV ERROR " << strerror(errno) << std::endl; // DEBUG
            }
            break;
        }
    }
    if (clients[i].response.getResponseCode() == ENTITYTOOLARGE)
    {
        clients[i].writeBuffer = Utils::returnResponseHeader(clients[i]);
        clients[i].events = REQUEST;
        pollFds[i].events |= POLLOUT;
    }
    else if (clients[i].response.getContentLength() == clients[i].formData.size()
        || tempChunk != clients[i].response.getIsChunked())
    {
        clients[i].response.setFormData(clients[i].formData);
        clients[i].events = REQUEST;
        if (clients[i].response.getisCGI())
            CGIHandle(clients[i]);
    }
}

int WebServer::new_connection(size_t i)
{
    Server &server = w_servers[i];
    int clientFd = accept(server.serverFd, (sockaddr *)&server.address, (socklen_t *)&server.addrLen);
    if (clientFd < 0) { return clientFd; }

    setNonBlocking(clientFd);
    addClient(clientFd, POLLIN, i);
    return 1;
}

void WebServer::client_send(int i) {
    Clients &c = clients[i];
    if (c.writeOffset >= c.writeBuffer.size()) {
        pollFds[i].events &= ~POLLOUT;
        return;
    }

    size_t  toSend = c.writeBuffer.size() - c.writeOffset;
    ssize_t n      = ::send(c.fd,
                            c.writeBuffer.data() + c.writeOffset,
                            toSend,
                            0);
    if (n > 0) {
        c.writeOffset += n;
        if (c.writeOffset >= c.writeBuffer.size()) {
            Utils::print_response(c);
            c.clearClient();
            pollFds[i].events &= ~POLLOUT;
        }
    }
    else if (n == 0) {
        std::cout << "Client SEND disconnected" << std::endl;
        closeClient(i);
    }
    else {
        return;
    }
}

void WebServer::start() {

    while (true) {
        int events = poll(pollFds.data(), pollFds.size(), -1);
        if (events < 0) throw ServerExcp("Poll Error");

        for (int i = pollFds.size() - 1; i >= 0; i--) {
            short re = pollFds[i].revents;
            if (re & (POLLHUP | POLLERR | POLLRDHUP)) {
                if (re & POLLERR)
                    printError(pollFds[i].fd);
                closeClient(i);
                continue;
            }

            if (i < serverSize && (re & POLLIN)) {
                new_connection(i);
                continue;
            }

            if (i > 0 && (re & POLLIN)) {
                if (clients[i].events == WAIT_FORM) {
                    readFormData(i);
                    if (clients[i].response.getResponseCode() == ENTITYTOOLARGE)
                        continue;
                } else {
                    ServerResponse(clients[i]);
                }

                if (!clients[i].writeBuffer.empty() && clients[i].events != WAIT_FORM)
                    pollFds[i].events |= POLLOUT;
            }

            if (i > 0 && (re & POLLOUT) && clients[i].events != WAIT_FORM
            && !clients[i].writeBuffer.empty())
                client_send(i);
            }
        }
}

const char *WebServer::ServerExcp::what() const throw()
{
    return excp.c_str();
}

WebServer::ServerExcp::ServerExcp(const std::string &err) : excp(err + ": Couldn't create the server!") {}
WebServer::ServerExcp::~ServerExcp() throw() {}
