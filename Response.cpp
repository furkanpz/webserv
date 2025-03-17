#include "Response.hpp"

std::string Response::getRequestType(void) const
{
    return requestType;
}

void Response::setRequestType(std::string _requestType)
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

Response::Response() : requestType(""), responseCode(-1), content("")
{
    
}