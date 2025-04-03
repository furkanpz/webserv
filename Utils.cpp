#include "webserv.hpp"
#include "Utils.hpp"

std::string Utils::intToString(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string Utils::readFile(const std::string &fileName, Response &response, int code)
{
    std::ifstream file(fileName.c_str());
    if (!file) {
        size_t dotPos = fileName.find_last_of(".");
        if (dotPos == std::string::npos) {
            response.setResponseCode(NOTFOUND);
            std::ifstream nf("notFound.html");
            std::stringstream buffer;
            if (nf) {
                buffer << nf.rdbuf();
                return buffer.str();
            }
            return "<h1>404 Not Found</h1>";
        }
        response.setResponseCode(code);
        return "";
    }
    else if (fileName.find("cgi-bin") == 0)
    {
        size_t temp = fileName.find("/");
        response.setisCGI(true);
        response.setResponseCode(code);
        return "";
    }
    
    response.setResponseCode(code);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string get_content_type(const std::string& http_buffer) {
    std::istringstream stream(http_buffer);
    std::string line;
    
    while (std::getline(stream, line)) {
        size_t pos = line.find("Content-Type:");
        if (pos != std::string::npos) {
            return line.substr(pos + 14);
        }
    }
    return "";
}

size_t Utils::getContentLenght(std::string request, Response &response)
{
    size_t  contentLength = 0;
    char    buffer[10240] = {0};
    long    bytesRead;

    if (response.getRequestType() == POST)
    {
        size_t pos = request.find("Content-Length:");
        if (pos != std::string::npos) {
            std::istringstream iss(request.substr(pos));
            std::string temp;
            iss >> temp >> contentLength;
        }
        std::cout << contentLength << std::endl;
    }
    return (contentLength);
}
bool Utils::waitPoll(int eventFd)
{
    struct pollfd   pfd;
    int             pollTimeout = 5000;
    int             pollResult;
    
    pfd.fd = eventFd;
    pfd.events = POLLIN;
    pollResult = poll(&pfd, 1, pollTimeout);
    if (pollResult > 0) {
        if (pfd.revents & POLLIN) {
            return true;
        }
    } else if (pollResult == 0) { // DEBUG
        std::cerr << "Poll timeout: İstemciden veri alınamadı." << std::endl;
    } else {
        std::cerr << "Poll hatası: " << strerror(errno) << std::endl;
    }
    return false;
}


void Utils::getFormData(std::string request, std::string body, Response &response, int eventFd)
{
    int contentLength = response.getContentLength();
    int bytesRead;
    char buffer[10240] = {0};
    while (body.length() < (size_t)contentLength) {
        bytesRead = recv(eventFd, buffer, sizeof(buffer) - 1, 0);
        buffer[bytesRead] = '\0';
        std::cout << buffer << std::endl;
        if (bytesRead <= 0)
        {
            if (waitPoll(eventFd))
                continue;
            break;
        }
        body.append(buffer, bytesRead);
        response.setContentTypeForPost(body);
    }
}


int Utils::countSeperator(const std::string &buffer, const std::string &target) {
    int count = 0;
    size_t pos = buffer.find(target);

    while (pos != std::string::npos) { 
        count++;
        pos = buffer.find(target, pos + target.length()); 
    }
    return count;
}

void Utils::doubleSeperator(std::string key, std::string &buffer,
    Response &response, int eventFd)
{
    std::string target = "=";
    size_t firstPos = key.find(target);
    if (firstPos == std::string::npos)
        return ;
    std::string seperator = key.substr(firstPos + 1);
    if (countSeperator(buffer, seperator) > 1)
    {
        size_t firstIndex = buffer.find(seperator, buffer.find(seperator) + 1);
        if (firstIndex != std::string::npos)
        {
            std::string temp = buffer.substr(firstIndex - 2);
            if (buffer.substr(firstIndex - 2).length() != response.getContentLength())
               getFormData(buffer, temp, response, eventFd);
            else
                response.setContentTypeForPost(temp);
        }
    }
    else
        getFormData(buffer, "", response, eventFd);
    if (response.getContentTypeForPost().length() != response.getContentLength()) // DEBUG İÇİN
        std::cout << "EKSİK KAYDETTİ! " << response.getContentTypeForPost().length()  << std::endl;

}

void Utils::parseContent(std::string &buffer, Response &response, int eventFd)
{
    std::string request(buffer);
    
    response.setFile(getFileName(request, response));
    response.setContent(readFile(response.getFile(), response));
    response.setcontentType(get_content_type(request));
    response.setContentLength(getContentLenght(request, response));
    if (request.find("GET ") == 0)
    response.setRequestType(GET);
    else if(request.find("POST ") == 0)
    {
        response.setRequestType(POST);
        Utils::doubleSeperator(response.getcontentType() , buffer, response, eventFd);
    }
    else if (request.find("DELETE ") == 0)
        response.setRequestType(DELETE);
}


std::string Utils::getFileName(std::string request, Response &response)
{
    size_t pos = request.find(" ");
    if (pos == std::string::npos) {
        return "";
    }

    size_t start = pos + 1;
    size_t end = request.find(" ", start);
    if (end == std::string::npos) {
        return "";
    }

    std::string path = request.substr(start, end - start);
    if (path == "/")
    return "index.html";
    
    if (path[0] == '/')
    path = path.substr(1);
    
    return path;
}
