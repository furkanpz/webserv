#pragma once
#include "webserv.hpp"
#include "Response.hpp"
#include "WebServer.hpp"
class Utils
{
    public:
        static std::string intToString(int num);
        static std::string readFile(const std::string& fileName, Response &response, int code = OK);
        static void parseContent(std::string &buffer, Response &response, Clients &client);
        static std::string getFileName(std::string request, Response &response);
        static size_t getContentLenght(std::string request, Response &response);
        static void doubleSeperator(std::string key, std::string &buffer, Response &response, Clients &client);
        static void getFormData(std::string request, Response &response, Clients &client);
        static bool waitPoll(int eventFd);
        static int countSeperator(const std::string &buffer, const std::string &target);
        static void directlyFormData(std::string body, Response &response, int eventFd);

};