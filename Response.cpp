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

Response::Response() : requestType(-1), responseCode(-1), content("")
{
    
}