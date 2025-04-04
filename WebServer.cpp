#include "WebServer.hpp"
#include "Utils.hpp"
#include "Response.hpp"
#include "sys/wait.h"

static const std::string methods[3] = {"GET", "POST", "DELETE"};

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

void WebServer::CGIHandle(Clients &client, Response &res)
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
        if (res.getRequestType() == POST) {
            setenv("CONTENT_TYPE", res.getcontentType().c_str(), 1);
            setenv("CONTENT_LENGTH", Utils::intToString(res.getContentLength()).c_str(), 1);
        }
        setenv("REQUEST_METHOD", (res.getRequestType() == POST ? "POST" : "GET"), 1);
        close(fd_out[0]);  
        dup2(fd_out[1], 1); 
        close(fd_out[1]);


        close(fd_in[1]); 
        dup2(fd_in[0], 0); 
        close(fd_in[0]);

        std::string scriptFile = res.getFile();
        #ifdef __APPLE__
            char *argv[] = { (char *)"python3", (char *)scriptFile.c_str(), NULL };
            execve("/usr/bin/python3", argv, environ);
        #else
            char *argv[] = { (char *)"python3.10", (char *)scriptFile.c_str(), NULL };
            execve("/bin/python3.10", argv, environ);
        #endif
        std::cerr << "ERROR " << std::endl;
        exit(1);
    }
    else {
        close(fd_out[1]); 
        close(fd_in[0]);
        if (res.getRequestType() == POST) 
            write(fd_in[1], res.getContentTypeForPost().c_str(), res.getContentTypeForPost().size());
        close(fd_in[1]); 
        char buffer[55555];
        std::string output;
        int bytesRead;
        while ((bytesRead = read(fd_out[0], buffer, sizeof(buffer))) > 0)
            output.append(buffer, bytesRead);
        close(fd_out[0]);
        waitpid(pid, NULL, 0);
        std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " +
                             Utils::intToString(output.size()) + "\r\n\r\n";
        std::string response = header + output;
        std::cout << client.response.getContentLength() << std::endl;
        client.client_send(client.fd, response.c_str(), response.size());
    }
}

WebServer::WebServer(const std::string &host, int port) : Port(port), Host(host) {
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
    for (int x = 0; x < pollFds.size(); x++)
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

bool WebServer::CheckResponse(Clients &client, std::string &headers, Response &res)
{
    if (headers.find("Expect: 100-continue") != std::string::npos) {
        std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
        // client.client_send(client.fd, continueResponse.c_str(), continueResponse.size());
        // if (Utils::waitPoll(eventFd))
                // Utils::directlyFormData("", res, eventFd);
        Utils::parseContent(headers, res, client);
    }
    else
        Utils::parseContent(headers, res, client);
    if (res.getisCGI())
    {
        if (client.events == WAIT_FORM)
        {
            std::cout << "WAIT_POLL" << std::endl;
            return true;
        }
        else
            return CGIHandle(client, res), true;
    }
    return false;
}

void WebServer::ServerResponse(Clients &client)
{
    std::string headers;
    char        buffer[10240] = {0};
    int         bytesRead;
    int         contentLength = 0;
    Response    &res = client.response;
    
    if (client.events == REQUEST)
    {
        while ((bytesRead = recv(client.fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
            headers.append(buffer, bytesRead);
            if (headers.find("\r\n\r\n") != std::string::npos)
                break;
        }
    }
    if (CheckResponse(client, headers, res))
        return;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + 
                        Utils::intToString(res.getContent().length()) + "\r\n\r\n" + res.getContent();
    client.client_send(client.fd, response.c_str(), response.size());
    // std::cout //<< methods[MAX_INT + res.getRequestType()] 
    //            << "Response : " << res.getFile() << " Response code: " << res.getResponseCode() << std::endl;

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
    std::cout << "BİTTİ ÇIKTI GİTTİ" << std::endl;
    close(clients[index].poll->fd);
    clients.erase(clients.begin() + index);
    pollFds.erase(pollFds.begin() + index);
}

void WebServer::start() {
    signal(SIGPIPE, SIG_IGN);
    std::cout << "Server listening on " << Host << ":" << Port << "..." << std::endl;

    while (true) {
        int events = poll(pollFds.data(), pollFds.size(), -1);
        std::cout <<"event: " <<  events << std::endl;
        if (events < 0) {
            throw ServerExcp("Poll Error");
        }
        for (int i = 0; i < pollFds.size(); i++) {
            if (i == 0)
            {
                if (pollFds[i].revents & POLLIN) {
                    int clientFd = accept(serverFd, (sockaddr *)&address, (socklen_t *)&addrLen);
                    if (clientFd < 0) { continue; }

                    std::cout << "New client connected: " << clientFd << std::endl;
                    setNonBlocking(clientFd);
                    addClient(clientFd, POLLIN);
                }
            }
            else
            {
                if (pollFds[i].revents & POLLIN)
                {
                    if (clients[i].events == WAIT_FORM)
                    {
                        std::cout << "BEFORE FORMDATA : " << clients[i].formData.size() << std::endl;
                        while (true)
                        {
                            char buffer[10240];
                            int bytesRead = recv(clients[i].fd, buffer, sizeof(buffer), 0);
                            if (bytesRead > 0) {
                                clients[i].formData.append(buffer, bytesRead);
                            }
                            else break;
                        }
                        std::cout << "AFTER FORMDATA : " << clients[i].formData.size() << std::endl;
                        std::cout << "CONTENT-LENGTH : " <<  clients[i].response.getContentLength() << std::endl;
                        if (clients[i].response.getContentLength() == clients[i].formData.size())
                        {
                            clients[i].events = NONE;
                            clients[i].response.setContentTypeForPost(clients[i].formData);
                        }
                    }
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