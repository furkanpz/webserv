#include "WebServer.hpp"
#include "Utils.hpp"

WebServer::WebServer(const std::string &host, int port) : Port(port), Host(host) {
    
    
    addrLen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons(Port);
    
    struct addrinfo hints, *res;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(host.c_str(), Utils::intToString(Port).c_str(), &hints, &res);
    if (status != 0) {freeaddrinfo(res); throw ServerExcp();}

    serverFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (serverFd == 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp();}
    if (bind(serverFd, res->ai_addr, res->ai_addrlen) < 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp();}
    freeaddrinfo(res);
    if (listen(serverFd, SOMAXCONN) < 0) {close(serverFd); throw ServerExcp();}

    setNonBlocking(serverFd);

    pollFd = POLLER();
    if (pollFd == -1){close(serverFd); throw ServerExcp();
}
    EVENT_STRUCT event;
    #ifdef __APPLE__
        EV_SET(&event, serverFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        kevent(pollFd, &event, 1, NULL, 0, NULL);
    #else
        event.data.fd = serverFd;
        event.events = EPOLLIN;
        epoll_ctl(pollFd, EPOLL_CTL_ADD, serverFd, &event);
    #endif
}


WebServer::~WebServer()
{
    close(serverFd);
    close(pollFd);
}

void WebServer::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void WebServer::ServerResponse(int eventFd)
{
    char buffer[1024] = {0};
    int bytesRead = read(eventFd, buffer, sizeof(buffer));
    if (bytesRead <= 0) {
        close(eventFd);
    } else {
        std::cout << "Received request:\n" << buffer << std::endl;
        std::string content = Utils::readFile("index.html"); 
        std::string contentLengthStr = Utils::intToString(content.length());
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + 
                            contentLengthStr + "\r\n\r\n" + content;
        send(eventFd, response.c_str(), response.length(), 0);
        close(eventFd);
    }
}

void WebServer::start() {
    std::cout << "Server listening on " << Host << ":" << Port << "...\n";
    EVENT_STRUCT events[MAX_EVENTS];

    while (true) {
        #ifdef __APPLE__
            int eventCount = kevent(pollFd, NULL, 0, events, MAX_EVENTS, NULL);
        #else
            int eventCount = epoll_wait(pollFd, events, MAX_EVENTS, -1);
        #endif
        for (int i = 0; i < eventCount; i++) {
            #ifdef __APPLE__
                int eventFd = events[i].ident;
            #else
                int eventFd = events[i].data.fd;
            #endif
            if (eventFd == serverFd) 
            {
                int clientFd = accept(serverFd, (struct sockaddr*)&address, (socklen_t*)&addrLen);
                if (clientFd < 0) continue;
                setNonBlocking(clientFd);
                EVENT_STRUCT event;
                #ifdef __APPLE__
                    EV_SET(&event, clientFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(pollFd, &event, 1, NULL, 0, NULL);
                #else
                    event.data.fd = clientFd;
                    event.events = EPOLLIN;
                    epoll_ctl(pollFd, EPOLL_CTL_ADD, clientFd, &event);
                #endif
            } 
            else
                ServerResponse(eventFd);
        }
    }
}

const char *WebServer::ServerExcp::what() const throw()
{
    return "Couldn't create the server!";
}
