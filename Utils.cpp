#include "webserv.hpp"
#include "Utils.hpp"

std::string Utils::intToString(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string Utils::readFile(const std::string &fileName)
{
    std::ifstream file(fileName.c_str());
    if (!file) {
        return "notFound.html";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Utils::parseContent(char *buffer)
{
     std::string request(buffer);
     
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
     std::string url = request.substr(start + 1, end - start - 1);
     return url;
}