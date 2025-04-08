#pragma once

#include "webserv.hpp"

class Response{
    private:
        std::string contentTypeForPost;
        std::string file;
        int requestType;
        int responseCode;
        std::string content;
        bool isCGI;
        size_t ContentLenght;
        std::string contentType;
        bool isChunked;
    public:
        Response();

        int getRequestType(void) const;
        void setRequestType(int _requestType);

        std::string getcontentType(void) const;
        void setcontentType(std::string _type);


        size_t getContentLength(void) const;
        void setContentLength(size_t _Lenght);

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