#pragma once

#include "webserv.hpp"
#include "Response.hpp"
#include "Utils.hpp"
#include "Clients.hpp"


class Utils
{
    private:
        static void ChunkedCompleted(Clients &client, std::string &result);
    public:
        static std::string intToString(int num);
        static void parseContent(std::string &buffer, Clients &client);
        static std::string getFileName(std::string request);
        static size_t getContentLenght(std::string request, Response &response);
        static void doubleSeperator(std::string key, std::string &buffer, Clients &client);
        static int countSeperator(const std::string &buffer, const std::string &target);
        static void print_response(Clients &client);
        static void getBufferFormData(std::string &buffer, Clients &client);
        static bool isDirectory(const std::string& path);
        static void parseChunked(Clients &client, std::string &Body);
        static void parseChunked_FT(Clients &client, std::string &Body, int Type);
        static std::string returnResponseHeader(Clients &client);
        static std::vector<std::string> split(const std::string &s, char delimiter);
        static std::string Spacetrim(const std::string &s);
        static std::string readFile(const std::string &fileName, Response &response, Clients &client, int code = OK);
        static std::string returnErrorPages(Response &response, int ErrorType, Clients &Client);
        static std::string generateAutoIndex(const std::string& path, const std::string& requestPath, Clients &client);
        static bool wait_with_timeout(pid_t pid, int timeout_seconds);
};
