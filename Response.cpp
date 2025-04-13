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
    int start = 0;
    bool flag_matched = false;
    int matched = -1;
    int root_index = -1;
    std::vector<std::string> parts = Utils::split(_file, '/');
    if (parts[0].empty()) // istegin basinda / var demektir;
        parts.erase(parts.begin());
    std::string united;
    std::vector<std::string> united_parts;
    while (start < parts.size())
    {
        united += "/";
        united += parts[start];
        united_parts.push_back(united);
        start++;
    }
    for (int i = 0; i < server.locations.size(); i++)
    {
        if (server.locations[i].path == united)
        {
            flag_matched = true;
            matched = i;
        }
        if (server.locations[i].path == "/")
        {
            root_index = i;
        }
    }
    std::string ret;
    if (flag_matched)
    {
        ret = "/" + server.locations[matched].path + parts[parts.size() - 1];
    }
    else
    {
        int decrease = 0;
        int partsize = parts.size();
        std::cout << partsize << std::endl;
        for (int i = 0; i < united_parts.size(); i++)
        {
            for (int d = 0; d < server.locations.size(); d++)
            {
                if (!server.locations[d].path.compare(united_parts[i]))
                {
                    flag_matched = true;
                    matched = d;
                    break;
                }
            }
            if (flag_matched)
            {
                ret = "/" + server.locations[matched].root + "/" + parts[parts.size() - 1];
                break;
            }
        }
    }
    if (!flag_matched)
      ret = "/" + server.locations[root_index].root + parts[parts.size() - 1];
    std::cout << ret << std::endl;
    file = ret;
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
