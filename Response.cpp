#include "Response.hpp"

int Response::getRequestType(void) const
{
    return requestType;
}

void Response::setRequestType(int _requestType)
{
    requestType = _requestType;
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



Response::Response() : requestType(-1), responseCode(-1), content(""), isCGI(false)
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
