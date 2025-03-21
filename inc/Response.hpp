#pragma once

#include "webserv.hpp"

class Response{
    private:
        int requestType;
        int responseCode;
        std::string content;
        std::string file;
    public:
    Response();

    int getRequestType(void) const;
    void setRequestType(int _requestType);

    int getResponseCode(void) const;
    void setResponseCode(int _responseCode);

    std::string getContent(void) const;
    void setContent(std::string _content);

    std::string getFile(void) const;
    void setFile(std::string _file);
};