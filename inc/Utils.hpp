#pragma once
#include "webserv.hpp"
#include "Response.hpp"

class Utils
{
    public:
        static std::string intToString(int num);
        static std::string readFile(const std::string& fileName, Response &response, int code = OK);
        static void parseContent(char *buffer, Response &response);
        static std::string getFileName(std::string request, Response &response);

};