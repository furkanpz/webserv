#pragma once

#include "webserv.hpp"
#include "Response.hpp"
#include "Utils.hpp"
#include "Clients.hpp"

class Utils
{
    public:
        static std::string intToString(int num);
        static std::string readFile(const std::string& fileName, Response &response, int code = OK);
        static void parseContent(std::string &buffer, Clients &client);
        static std::string getFileName(std::string request, Response &response);
        static size_t getContentLenght(std::string request, Response &response);
        static void doubleSeperator(std::string key, std::string &buffer, Clients &client);
        static void getFormData(std::string request, Response &response, Clients &client);
        static int countSeperator(const std::string &buffer, const std::string &target);
        static void directlyFormData(std::string body, Response &response, int eventFd);
        static void print_response(Response &response);
        static void getBufferFormData(std::string &buffer, Clients &client);
        static bool isDirectory(const std::string& path);

};