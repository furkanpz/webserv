#include "Response.hpp"

int Response::getRequestType(void) const
{
    return requestType;
}

void Response::setRequestType(int _requestType)
{
    requestType = _requestType;
}

void Response::setContentLength(size_t _Lenght)
{
    ContentLenght = _Lenght;
}

size_t Response::getContentLength(void) const
{
    return ContentLenght;
}

int Response::getResponseCode(void) const
{
    return responseCode;
}

void Response::setResponseCode(int _responseCode)
{
    responseCode = _responseCode;
}

std::string Response::getContent(void) const
{
    return content;
}

void Response::setContent(std::string _content)
{
    content = _content;
}

std::string Response::getFile(void) const
{
    return file;
}

void Response::setFile(std::string _file)
{
    file = _file;
}

bool Response::getisCGI(void) const
{
    return isCGI;
}

void Response::setisCGI(bool _tf)
{
    isCGI = _tf;
}

std::string Response::getcontentType(void) const
{
    return (contentType);
}

void Response::setcontentType(std::string _type)
{
    contentType = _type;
}


Response::Response() : contentTypeForPost(""), file(""), requestType(NONE), 
    responseCode(-1), content(""), isCGI(false), ContentLenght(0),
    contentType(""), isChunked(false)
{
    
}

std::string Response::getContentTypeForPost(void) const
{
    return contentTypeForPost;
}

void Response::setContentTypeForPost(std::string _contentTypeForPost)
{
    contentTypeForPost = _contentTypeForPost;
}
bool Response::getIsChunked() const
{
    return isChunked;
}

void Response::setIsChunked(bool _val)
{
    this->isChunked = _val;
}