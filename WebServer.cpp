#include "WebServer.hpp"
#include "Utils.hpp"
    
WebServer::WebServer(const std::string &host, int port) : Port(port), Host(host) {
    addrLen = sizeof(address);
    address.sin_family = AF_INET;
    address.sin_port = htons(Port);
    if (host == "localhost") {
        address.sin_addr.s_addr = INADDR_ANY;
    } else {
        address.sin_addr.s_addr = inet_addr(host.c_str());
    }

    serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, SOMAXCONN) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    setNonBlocking(serverFd);

    #ifdef __APPLE__
        pollFd = POLLER();
    #else
        pollFd = POLLER(0);
    #endif
    if (pollFd == -1) {
        perror("Poller creation failed");
        exit(EXIT_FAILURE);
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


WebServer::~WebServer() {
    close(serverFd);
    close(pollFd);
}

void WebServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
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
                    if (eventFd == serverFd) {
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
            } else {
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
        }
    }
}
