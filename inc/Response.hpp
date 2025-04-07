#pragma once

#include "webserv.hpp"

class Response{
    private:
        int requestType;
        int responseCode;
        std::string contentTypeForPost;
        std::string content;
        std::string file;
        int ContentLenght;
        std::string contentType;
        bool isCGI;
        bool isChunked;
    public:
    Response();

    int getRequestType(void) const;
    void setRequestType(int _requestType);

    std::string getcontentType(void) const;
    void setcontentType(std::string _type);


    int getContentLength(void) const;
    void setContentLength(int _Lenght);

    int getResponseCode(void) const;
    void setResponseCode(int _responseCode);

    std::string getContent(void) const;
    void setContent(std::string _content);

    std::string getFile(void) const;
    void setFile(std::string _file);

    bool getisCGI(void) const;
    void setisCGI(bool _tf);

    bool getIsChunked() const;
    void setIsChunked(bool _val);

    std::string getContentTypeForPost(void) const;
    void setContentTypeForPost(std::string _contentTypeForPost);
};