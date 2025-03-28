#pragma once
#include "webserv.hpp"
#include "Response.hpp"

class Utils
{
    public:
        static std::string intToString(int num);
        static std::string readFile(const std::string& fileName, Response &response, int code = OK);
        static void parseContent(std::string &buffer, Response &response, int eventFd);
        static std::string getFileName(std::string request, Response &response);
        static size_t getContentLenght(std::string request, Response &response);
        static void doubleSeperator(std::string key, std::string &buffer, Response &response, int eventFd);
        static void getFormData(std::string request, std::string body, Response &response, int eventFd);
};