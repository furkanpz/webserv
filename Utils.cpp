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
    std::cout << "filename: " + fileName << std::endl;
    std::ifstream file(fileName.c_str());
    if (!file) {
        if (!isDirectory("/" + fileName)) {
            response.setResponseCode(NOTFOUND);
            std::ifstream nf("www/notFound.html");
            std::stringstream buffer;
            if (nf) {
                buffer << nf.rdbuf();
                return buffer.str();
            }
            response.setResponseCode(FORBIDDEN);
            return "<h1>403 FORBIDDEN</h1>";
        }
        else
        {
            std::string _fileName = "/" + fileName;
            std::string possible_path = fileName + "/index.html";
            std::cout << "eğer klasorse" << std::endl;
            if (access(_fileName.c_str(), X_OK) != 0)
            {
                std::cout << "okuma yetkisi yoksa" << std::endl;

                response.setResponseCode(FORBIDDEN);
                return "<h1>403 FORBIDDEN</h1>";
            }
            else if (!access(possible_path.c_str(), R_OK))
            {
                std::cout << "index.html varsa" << std::endl;
                response.setResponseCode(code);
                return possible_path;
            }
            
        }
        response.setResponseCode(code);
        return "";
    }
    else if (fileName.find("cgi-bin") != std::string::npos)
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

    size_t pos = request.find("Content-Length:");
    if (pos != std::string::npos) {
        std::istringstream iss(request.substr(pos));
        std::string temp;
        iss >> temp >> contentLength;
    }
    return (contentLength);
}

void Utils::directlyFormData(std::string body, Response &response, int eventFd)
{
    char buffer[10240];
    int bytesRead = recv(eventFd, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        body.append(buffer, bytesRead);
    }
    response.setContentTypeForPost(body);
}

void Utils::getFormData(std::string request, Response &response, Clients &client)
{
    int contentLength = response.getContentLength();
    int bytesRead;
    char buffer[10240] = {0};
    std::string body;
    while (body.length() < (size_t)contentLength) {
        bytesRead = recv(client.fd, buffer, sizeof(buffer) - 1, 0);;
        if (bytesRead > 0)
            body.append(buffer, bytesRead);
        else
            break;
    }
    client.formData.append(body);
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

void Utils::doubleSeperator(std::string key, std::string &buffer, Clients &client)
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
            if (temp.length() == client.response.getContentLength())
                client.response.setContentTypeForPost(temp);
            else
                client.formData.append(temp);
        }
    }
    if (client.response.getContentTypeForPost().length() != client.response.getContentLength()) // DEBUG İÇİN
        client.events = WAIT_FORM;
}

void Utils::getBufferFormData(std::string &buffer, Clients &client)
{
    std::string contentType = client.response.getcontentType().substr(0, client.response.getcontentType().find(";"));
    if (!contentType.find("multipart/form-data"))
        Utils::doubleSeperator(client.response.getcontentType() , buffer, client);
    else if (!contentType.find("application/x-www-form-urlencoded"))
    {
        if (buffer.find("\r\n\r\n") != std::string::npos)
            client.response.setContentTypeForPost(buffer.substr(buffer.find("\r\n\r\n") + 4));
    }
}

void Utils::parseContent(std::string &buffer, Clients &client)
{
    std::string request(buffer);
    Response &response = client.response;
    
    if (client.events == REQUEST && client.response.getRequestType() == NONE)
    {
        response.setFile(getFileName(request, response));
    std::cout << "DEBUG: " << response.getFile() << std::endl;
        response.setContent(readFile(response.getFile(), response));
        response.setcontentType(get_content_type(request));
        response.setContentLength(getContentLenght(request, response));
        if (request.find("DELETE ") == 0)
        response.setRequestType(DELETE);
        else if(request.find("POST ") == 0)
        {
            response.setRequestType(POST);
            getBufferFormData(buffer, client);
        } 
        else if (request.find("GET ") == 0)
            response.setRequestType(GET);
        else
            response.setRequestType(NONE);
    }
}


bool Utils::isDirectory(const std::string& path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        return S_ISDIR(s.st_mode);
    }
    return false;
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
    return "www/index.html";
    
    if (path[0] == '/')
        path = path.substr(1);
    if (isDirectory(path))
        return path;
    return "www/" + path;
}

void Utils::print_response(Response &response)
{
    if (response.getRequestType() == NONE)
        return ;
    std::string meth("[" + methods[MAX_INT - response.getRequestType()] + "]");
    std::cout << meth << std::setw(3)
               << " /" << response.getFile() << " " << response.getResponseCode() << std::endl;
}