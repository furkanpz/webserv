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

    response.setResponseCode(code);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}


void Utils::parseContent(char *buffer, Response &response)
{
    std::string request(buffer);
     
    response.setFile(getFileName(request, response));
    response.setContent(readFile(response.getFile(), response));
    if (request.find("GET ") == 0)
        response.setRequestType(GET);
    else if(request.find("POST ") == 0)
        response.setRequestType(POST);
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
