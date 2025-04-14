#pragma once

#include "webserv.hpp"


class Response{
    private:
        std::string contentTypeForPost;
        std::string file;
        std::string pureLink;
        int requestType;
        int responseCode;
        std::string content;
        bool isCGI;
        std::string cgiPath;
        std::string cgiExtension;
        size_t ContentLenght;
        std::string contentType;
        bool isChunked;
        std::string responseCodestr;
        bool methodNotAllowed;
        
    public:
        Response();

        int getRequestType(void) const;
        void setRequestType(int _requestType);

        std::string getcontentType(void) const;
        void setcontentType(std::string _type);

        bool getMethodNotAllowed(void) const;
        void setMethodNotAllowed(bool _tf);

        size_t getContentLength(void) const;
        void setContentLength(size_t _Lenght);

        int getResponseCode(void) const;
        void setResponseCode(int _responseCode);

        std::string getContent(void) const;
        void setContent(std::string _content);

        std::string getFile(void) const;
        void setFile(std::string _file, Server &server);

        bool getisCGI(void) const;
        void setisCGI(bool _tf);

        bool getIsChunked() const;
        void setIsChunked(bool _val);

        std::string getContentTypeForPost(void) const;
        void setContentTypeForPost(std::string _contentTypeForPost);

        std::string getResponseCodestr(void) const;

        std::string getCgiPath(void) const;
        void setCgiPath(std::string _cgiPath);

        std::string getCgiExtension(void) const;
        void setCgiExtension(std::string _cgiExtension);

        std::string getPureLink(void) const;
        void setPureLink(std::string _pureLink);
    };