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

void WebServer::CGIHandle(int clientFd, const std::string &scriptPath,
        const std::string &queryString, const std::string &method,
        const std::string &body)
{
    extern char **environ;
    int fd[2];

    if (pipe(fd) == -1)
        return ;
    
    pid_t pid = fork();
    if (pid < 0) { close(fd[0]); close(fd[1]); }
    else if (pid == 0)
    {
        close(fd[0]);
        setenv("REQUEST_METHOD", method.c_str(), 1);
        setenv("QUERY_STRING", queryString.c_str(), 1);
        if (method == "POST") {
            setenv("CONTENT_LENGTH", std::to_string(body.length()).c_str(), 1);
        }
        dup2(fd[1], 1);
        close(fd[1]);
        char *argv[] = { (char *)"python3", (char *)scriptPath.c_str(), nullptr };
        execve("/usr/bin/python3", argv, environ);
        exit(1);
    }
    else {
        close(fd[1]);
        char buffer[1024];
        std::string output;
        int bytesRead;
        while ((bytesRead = read(fd[0], buffer, sizeof(buffer))) > 0) {
            output.append(buffer, bytesRead);
        }
        close(fd[0]);

        // Çocuk sürecin bitmesini bekleyin.
        waitpid(pid, NULL, 0);

        // HTTP yanıtını oluşturun ve istemciye gönderin.
        std::string header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " +
                             std::to_string(output.size()) + "\r\n\r\n";
        std::string response = header + output;
        send(clientFd, response.c_str(), response.length(), 0);
        close(clientFd);
    }
}

WebServer::WebServer(const std::string &host, int port) : Port(port), Host(host) {
    serverFd = SocketCreator(host);
    if (serverFd == 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp("Socket Error");}
    if (bind(serverFd, res->ai_addr, res->ai_addrlen) < 0) {freeaddrinfo(res); close(serverFd); throw ServerExcp("Bind Error");}
    freeaddrinfo(res);
    if (listen(serverFd, SOMAXCONN) < 0) {close(serverFd); throw ServerExcp("Listen Error");}

    setNonBlocking(serverFd);

    pollfd serverPollFd = {serverFd, POLLIN, 0};
    pollFds.push_back(serverPollFd);
}


WebServer::~WebServer()
{
    close(serverFd);
}

void WebServer::setNonBlocking(int fd)
{
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
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
        if (val.getisCGI() == true)
        {
            CGIHandle(eventFd, val.getFile(), "", "DENEMEE", "");
            return;
        }
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
                    std::cout << pollFds[i].fd << std::endl;
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