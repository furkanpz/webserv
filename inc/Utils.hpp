#pragma once
#include "webserv.hpp"

class Utils
{
    public:
        static std::string intToString(int num);
        static std::string readFile(const std::string& fileName);
        static std::string parseContent(char *buffer);

};