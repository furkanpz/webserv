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
        response.setResponseCode(code);
        return readFile("notFound.html", response, NOTFOUND);
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

     if (request.substr(start, end - start) == "/")
        return ("index.html");
     return (request.substr(start + 1, end - start - 1));
}
