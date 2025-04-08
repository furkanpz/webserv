#include "WebServer.hpp"
#include "Utils.hpp"
#include "Response.hpp"
#include "sys/wait.h"


int WebServer::SocketCreator(const std::string &host){
    addrLen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons(Port);

    std::memset(&first, 0, sizeof(first));

    first.ai_family = AF_INET;
    first.ai_socktype = SOCK_STREAM;
    first.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(host.c_str(), Utils::intToString(Port).c_str(), &first, &res);
    if (status != 0) {freeaddrinfo(res); throw ServerExcp("getaddrinfo Error");}
    return (socket(res->ai_family, res->ai_socktype, res->ai_protocol));
}

void WebServer::CGIHandle(Clients &client)
{
    extern char **environ;
    int fd_out[2]; 
    int fd_in[2];
    if (pipe(fd_out) == -1 || pipe(fd_in) == -1)
        return;

    pid_t pid = fork();
    if (pid < 0) { 
        close(fd_out[0]); close(fd_out[1]);
        close(fd_in[0]); close(fd_in[1]);
        return;
    }
    else if (pid == 0)
    {
        if (client.response.getRequestType() == POST) {
            setenv("CONTENT_TYPE", client.response.getcontentType().c_str(), 1);
            setenv("CONTENT_LENGTH", Utils::intToString(client.response.getContentLength()).c_str(), 1);
        }
        setenv("REQUEST_METHOD", (client.response.getRequestType() == POST ? "POST" : "GET"), 1);
        close(fd_out[0]);  
        dup2(fd_out[1], 1); 
        close(fd_out[1]);


        close(fd_in[1]); 
        dup2(fd_in[0], 0); 
        close(fd_in[0]);

        std::string scriptFile = client.response.getFile();
        #ifdef __APPLE__
            char *argv[] = { (char *)"python3", (char *)scriptFile.c_str(), NULL };
            execve("/usr/bin/python3", argv, environ);
        #else
            char *argv[] = { (char *)"python3.12", (char *)scriptFile.c_str(), NULL };
            execve("/bin/python3.12", argv, environ);
        #endif
        std::cerr << "ERROR " << std::endl;
        exit(1);
    }
    else {
        char        buffer[55555];
        std::string response;
        int         bytesRead;

        close(fd_out[1]); 
        close(fd_in[0]);

        if (client.response.getRequestType() == POST) 
            write(fd_in[1], client.response.getContentTypeForPost().c_str(), client.response.getContentTypeForPost().size());
        
        close(fd_in[1]); 
        while ((bytesRead = read(fd_out[0], buffer, sizeof(buffer))) > 0)
            response.append(buffer, bytesRead);
    
        close(fd_out[0]);
        waitpid(pid, NULL, 0);

        if (response.find("Bad Request\r\n") != std::string::npos)
            client.response.setResponseCode(BADREQUEST);
        
        Utils::print_response(client.response);
        client.client_send(client.fd, response.c_str(), response.size());
    }
}

WebServer::WebServer(const std::string &host, int port) : Host(host), Port(port) {
    serverFd = SocketCreator(host);
    if (serverFd == 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp("Socket Error");}
    int opt = 1;
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(serverFd);
        freeaddrinfo(res);
        throw ServerExcp("Setsockopt Error");
    }
    if (bind(serverFd, res->ai_addr, res->ai_addrlen) < 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp("Bind Error");}
    freeaddrinfo(res);
    if (listen(serverFd, SOMAXCONN) < 0) {close(serverFd); throw ServerExcp("Listen Error");}

    setNonBlocking(serverFd);
    addClient(serverFd, POLLIN);
}


WebServer::~WebServer()
{
    for (size_t x = 0; x < pollFds.size(); x++)
    {
        close(pollFds[x].fd);
    }
    close(serverFd);
}

void WebServer::setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
        throw ServerExcp("Fcntl Error");
}

bool WebServer::CheckResponse(Clients &client, std::string &headers)
{
    if (headers.find("Expect: 100-continue") != std::string::npos) {
        std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
        send(client.fd, continueResponse.c_str(), continueResponse.size(), 0);
    }

    Utils::parseContent(headers, client);
    // DEBUG
    //DEBUG
    if (client.response.getisCGI())
    {
        if (client.events == WAIT_FORM)
        {
            return true;
        }
        else
            return CGIHandle(client), true;
    }
    return false;
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
        while ((bytesRead = recv(client.fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            headers.append(buffer, bytesRead);
            if (headers.find("\r\n\r\n") != std::string::npos)
                break;
        }
    }
    if (CheckResponse(client, headers))
        return;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + 
                        Utils::intToString(client.response.getContent().length()) + "\r\n\r\n" + client.response.getContent();
    Utils::print_response(client.response);
    client.client_send(client.fd, response.c_str(), response.size());
}


void WebServer::addClient(int fd, short events)
{
    pollfd clientPollFd = {fd, events, 0};
    Clients newClient(clientPollFd, fd);
    pollFds.push_back(clientPollFd);
    clients.push_back(newClient);

}

void WebServer::closeClient(int index)
{
    close(clients[index].fd);
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
            if (tempChunk)
            {
                if (clients[i].formData.find("0\r\n\r\n") != std::string::npos) {
                    Utils::parseChunked(clients[i], clients[i].formData, 1);
                    }
            }
        }
        else break;
    }
    if (clients[i].response.getContentLength() == clients[i].formData.size()
        || tempChunk != clients[i].response.getIsChunked())
    {
        clients[i].response.setContentTypeForPost(clients[i].formData);
        if (clients[i].response.getisCGI())
            CGIHandle(clients[i]);
    }
}

int WebServer::new_connection()
{
    int clientFd = accept(serverFd, (sockaddr *)&address, (socklen_t *)&addrLen);
    if (clientFd < 0) { return clientFd; }

    setNonBlocking(clientFd);
    addClient(clientFd, POLLIN);
    return 1;
}

void WebServer::start() {
    std::cout << "Server listening on " << Host << ":" << Port << "..." << std::endl;

    while (true) {
        int events = poll(pollFds.data(), pollFds.size(), -1);
        if (events < 0) {
            throw ServerExcp("Poll Error");
        }
        for (size_t i = 0; i < pollFds.size(); i++) {
            if (i == 0 && (pollFds[i].revents && POLLIN) && new_connection() < 0)
                continue;
            else
            {
                if (pollFds[i].revents & POLLIN)
                {
                    if (clients[i].events == WAIT_FORM)
                        readFormData(i);
                    ServerResponse(clients[i]);
                }
                if (pollFds[i].revents & POLLOUT)
                    std::cout << "POLL OUT VERIYOR LAAAN" << std::endl;
                if (pollFds[i].revents & POLLHUP || pollFds[i].revents & POLLERR) 
                    closeClient(i);
            }
        }
    }
}

const char *WebServer::ServerExcp::what() const throw()
{
    return excp.c_str();
}

WebServer::ServerExcp::ServerExcp(const std::string &err) : excp(err + ": Couldn't create the server!") {}
WebServer::ServerExcp::~ServerExcp() throw() {}