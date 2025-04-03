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

void WebServer::CGIHandle(int clientFd, Response &res)
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
        send(clientFd, response.c_str(), response.length(), 0);
        // close(clientFd);
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

    pollfd serverPollFd = {serverFd, POLLIN, 0};
    pollFds.push_back(serverPollFd);
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

bool WebServer::CheckResponse(int eventFd, std::string &headers, Response &res)
{
    if (headers.find("Expect: 100-continue") != std::string::npos) {
        std::string continueResponse = "HTTP/1.1 100 Continue\r\n\r\n";
        send(eventFd, continueResponse.c_str(), continueResponse.size(), 0);
        if (Utils::waitPoll(eventFd))
                Utils::directlyFormData("", res, eventFd);
        Utils::parseContent(headers, res, eventFd);
    }
    else
        Utils::parseContent(headers, res, eventFd);
    if (res.getisCGI())
        return CGIHandle(eventFd, res), true;
    return false;
}

void WebServer::ServerResponse(int eventFd)
{
    Response    res;
    std::string headers;
    char        buffer[10240] = {0};
    int         bytesRead;
    int         contentLength = 0;

    while ((bytesRead = recv(eventFd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        headers.append(buffer, bytesRead);
        if (headers.find("\r\n\r\n") != std::string::npos)
            break;
    }
    std::cout << headers << std::endl;
    if (CheckResponse(eventFd, headers, res))
        return;
    std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + 
                        Utils::intToString(res.getContent().length()) + "\r\n\r\n" + res.getContent();
    send(eventFd, response.c_str(), response.length(), 0);
    std::cout //<< methods[MAX_INT + res.getRequestType()] 
               << "Response : " << res.getFile() << " Response code: " << res.getResponseCode() << std::endl;

}

void WebServer::start() {
    std::cout << "Server listening on " << Host << ":" << Port << "..." << std::endl;

    while (true) {
        int events = poll(pollFds.data(), pollFds.size(), -1);
        if (events < 0) {
            throw ServerExcp("Poll Error");
        }
        std::cout <<"event: " <<  events << std::endl;

        for (int i = 0; i < pollFds.size(); i++) {
            if (pollFds[i].revents & POLLIN) {
                if (pollFds[i].fd == serverFd) {
                    int clientFd = accept(serverFd, (sockaddr *)&address, (socklen_t *)&addrLen);
                    if (clientFd < 0) { continue; }

                    std::cout << "New client connected: " << clientFd << std::endl;
                    
                    pollfd clientPollFd = {clientFd, POLLIN, 0};
                    pollFds.push_back(clientPollFd);
                    setNonBlocking(clientFd);
                } 
                else 
                {
                    ServerResponse(pollFds[i].fd);
                    close(pollFds[i].fd);
                    pollFds.erase(pollFds.begin() + i);
                    i--; 
                }
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