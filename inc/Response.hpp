#pragma once

#include "webserv.hpp"

class Response{
    private:
        std::string requestType;
        int responseCode;
        std::string content;
    public:
    Response();

    std::string getRequestType(void) const;
    void setRequestType(std::string _requestType);

    int getResponseCode(void) const;
    void setResponseCode(int _responseCode);

    std::string getContent(void) const;
    void setContent(std::string _content);
};