#include "Response.hpp"
#include "Utils.hpp"
#include "Clients.hpp"


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

void Response::setFile(std::string _file, Server &server)
{
    if (_file.find('/') == std::string::npos)
    {
        file = _file;
        return;
    }

    int                         matchValues[4] = {false, -1, -1, -1};
    std::vector<std::string>    parts = Utils::split(_file, '/');
    int                         size[3] = {(int)parts.size(), 0, 0};
    std::vector<std::string>    united_parts;
    std::string                 united;

    for (int i = 0; i < size[0]; i++)
    {
        united += "/" + parts[i];
        united_parts.push_back(united);
    }
    if (size[0] == 0)
        united_parts.push_back("/");
    size[1] = (int)united_parts.size(); size[2] = server.locations.size();
    for (int i = size[1] - 1; i >= 0; i--)
    {
        if (!matchValues[0])
        {
            for (int d = 0; d < size[2]; d++)
            {
                if (matchValues[1] == -1 && !server.locations[d].path.compare("/"))
                    matchValues[1] = d;
                if (!server.locations[d].path.compare(united_parts[i]))
                {
                    matchValues[0] = true; matchValues[2] = d; matchValues[3] = i;
                    break;
                }
            }
        }
        if (matchValues[0])
        {
            // if (server.locations[matchValues[2]].root.empty())
            //     file = "/" + parts[parts.size() - 1]; // not found dönmeli veya parent root alınmalı!
            file += server.locations[matchValues[2]].root;
            break;
        }
    }
    if (!matchValues[0])
    {
        if (matchValues[1] == -1)
        {
            for (int i = 0; i < size[2]; i++)
            {
                if (!server.locations[i].path.compare("/"))
                {
                    file = server.locations[i].root; break;
                }
            }
        }
        else
            file = server.locations[matchValues[1]].root;
        if (matchValues[1] == -1)
            file = "";
    }   

    for (int i = matchValues[3] + 1; i < size[0]; i++)
        file += "/" + parts[i];
    std::cout << "File: " << file << std::endl;
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
    contentType(""), isChunked(false), responseCodestr("")
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

std::string Response::getResponseCodestr(void) const
{
    if (this->responseCode == 404)
        return "404 Not Found";
    else if (this->responseCode == 200)
        return "200 OK";
    else if (this->responseCode == 403)
        return "403 Forbidden";
    else if (this->responseCode == 400)
        return "400 Bad Request";
    else if (this->responseCode == 500)
        return "500 Internal Server Error";
    else if (this->responseCode == 413)
        return "413 Request Entity Too Large";
    else
        return "200 OK";
}
