#include "WebServer.hpp"
#include "Utils.hpp"

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


WebServer::WebServer(const std::string &host, int port) : Port(port), Host(host) {
    serverFd = SocketCreator(host);
    if (serverFd == 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp("Socket Error");}
    if (bind(serverFd, res->ai_addr, res->ai_addrlen) < 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp("Bind Error");}
    freeaddrinfo(res);
    if (listen(serverFd, SOMAXCONN) < 0) {close(serverFd); throw ServerExcp("Listen Error");}

    setNonBlocking(serverFd);

    pollFd = new pollfd[MAX_EVENTS];
    pollFd[0].fd = serverFd;
    pollFd[0].events = POLLIN;
}


WebServer::~WebServer()
{
    close(serverFd);
    delete[] pollFd;
}

void WebServer::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw ServerExcp("Fcntl Error");
    int test = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (test == -1)
        throw ServerExcp("Fcntl Error");
}

void WebServer::ServerResponse(int eventFd)
{
    Response val;
    char buffer[1024] = {0};
    int bytesRead = read(eventFd, buffer, sizeof(buffer));
    if (bytesRead <= 0) {
        close(eventFd);
        return ;
    } else {
        Utils::parseContent(buffer, val);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + 
                            Utils::intToString(val.getContent().length()) + "\r\n\r\n" + val.getContent();
        send(eventFd, response.c_str(), response.length(), 0);
        close(eventFd);
    }
     std::cout //<< methods[MAX_INT + val.getRequestType()] 
               << "Response : " << val.getFile() << " Response code: " << val.getResponseCode() << std::endl;

}

void WebServer::start() {
    std::cout << "Server listening on " << Host << ":" << Port << "..." << std::endl;

    while (true) {
        int events = poll(pollFd, MAX_EVENTS, -1);
        if (events < 0) {
            throw ServerExcp("Poll Error");
        }
        std::cout <<"event: " <<  events << std::endl;

        for (int i = 0; i < MAX_EVENTS; i++) {
            if (pollFd[i].revents & POLLIN) {
                if (pollFd[i].fd == serverFd) {
                    int clientFd = accept(serverFd, (sockaddr *)&address, (socklen_t *)&addrLen);
                    if (clientFd < 0) {
                        std::cout << "Accept Error: " << strerror(errno) << std::endl;
                        continue;
                    }

                    std::cout << "New client connected: " << clientFd << std::endl;

                    std::cout << "i size:" << i << std::endl;
                    bool added = false;
                    for (int j = 1; j < MAX_EVENTS; j++) {
                        if (pollFd[j].fd == 0) { 
                            std::cout << "j size:" << j << std::endl;
                            pollFd[j].fd = clientFd;
                            pollFd[j].events = POLLIN;
                            added = true;
                            break;
                        }
                    }
                    if (!added) {
                        std::cout << "Max clients reached, unable to accept new connections!" << std::endl;
                        close(clientFd);
                    } else {
                        setNonBlocking(clientFd); 
                    }

                } else {
                    ServerResponse(pollFd[i].fd);
                    pollFd[i].fd = 0;
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