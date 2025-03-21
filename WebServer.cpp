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

    pollFd = POLLER();
    if (pollFd == -1){close(serverFd); throw ServerExcp("Poll Error");}
    
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
        #ifndef __APPLE__
            epoll_ctl(pollFd, EPOLL_CTL_DEL, eventFd, NULL);
        #else
            EV_SET(&event, eventFd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
            kevent(pollFd, &event, 1, NULL, 0, NULL);
        #endif
            close(eventFd);
        return ;
    } else {
        Utils::parseContent(buffer, val);
        std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + 
                            Utils::intToString(val.getContent().length()) + "\r\n\r\n" + val.getContent();
        send(eventFd, response.c_str(), response.length(), 0);
        close(eventFd);
    }
    if (val.getResponseCode() == OK)
    {
        std::cout << "\033[32m" << methods[MAX_INT + val.getRequestType()] << "\033[32m: \033[32m" << val.getFile() << "\033[32m" << std::endl;
        std::cout << "\033[0m";
    }
    else
    {
        std::cout << "\033[31" << methods[MAX_INT + val.getRequestType()] << "\033[31: \033[31" << val.getFile() << "\033[31" << std::endl;
        std::cout << "\033[0m";
    }
}

void WebServer::start() {
    std::cout << "\033[35mServer listening on \033[35m" << Host << "\033[35m" << "\033[35m:\033[35m" << Port << "\033[35m" << "\033[35m...\033[35m" << std::endl;
    std::cout << "\033[0m";
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
    return excp.c_str();
}

WebServer::ServerExcp::ServerExcp(const std::string &err) : excp(err + ": Couldn't create the server!") {}
WebServer::ServerExcp::~ServerExcp() throw() {}