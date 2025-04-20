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
    pureLink = _file;
    if (_file.find('/') == std::string::npos)
    {
        file = _file;
        return;
    }
    else
        file = ".";
    int                         matchValues[3] = {false, -1, -1};
    std::vector<std::string>    parts = Utils::split(_file, '/');
    int                         size[3] = {(int)parts.size(), 0, 0};
    std::vector<std::string>    united_parts;
    std::string                 united;
    std::vector<std::string>::iterator   methodit;

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
                if (!server.locations[d].path.compare(united_parts[i]))
                {
                    if (cgiPath.empty() && !server.locations[d].cgi_path.empty())
                        cgiPath = server.locations[d].cgi_path;
                    if (cgiExtension.empty() && !server.locations[d].cgi_extension.empty())
                        cgiExtension = server.locations[d].cgi_extension;
                    matchValues[0] = true; matchValues[1] = d; matchValues[2] = i;
                    break;
                }
            }
        }
        if (matchValues[0])
        {
            file += server.locations[matchValues[1]].root;
            autoIndex = server.locations[matchValues[1]].autoindex;
            break;
        }
    }
    if (!matchValues[0])
    {
        if (!server.locations[server.rootLocation].root.empty())
            file += server.locations[server.rootLocation].root;
        else
            file = ""; // 404
        autoIndex = server.locations[server.rootLocation].autoindex;
        _methods = server.locations[server.rootLocation].methods;
    }
    else
    {
        _methods = server.locations[matchValues[1]].methods;
        std::vector<std::string>::iterator itend = server.locations[matchValues[1]].methods.end();
        if (requestType != NONE)
        {
            for (methodit = server.locations[matchValues[1]].methods.begin(); methodit != itend; methodit++)
            {
                if (*methodit == methods[MAX_INT - requestType])
                    break;
            }
            if (methodit != itend)
                responseCode = 200;
            else
                responseCode = 405;
        }
        else
            responseCode = 405;
    }
    for (int i = matchValues[2] + 1; i < size[0]; i++)
        file += "/" + parts[i];
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

Response::Response() : formData(""), file(""), requestType(NONE), 
    responseCode(-1), content(""), isCGI(false), cgiPath(""), cgiExtension(""),
    ContentLenght(0), contentType(""), isChunked(false), 
    responseCodestr(""), methodNotAllowed(false)
{
    
}

std::string Response::getFormData(void) const
{
    return formData;
}

void Response::setFormData(std::string _contentTypeForPost)
{
    formData = _contentTypeForPost;
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
    if (this->responseCode == 200)
        return "200 OK";
    else if (this->responseCode == 301)
        return "301 Moved Permanently";
    else if (this->responseCode == 302)
        return "302 Found";
    else if (this->responseCode == 303)
        return "303 See Other";
    else if (this->responseCode == 304)
        return "304 Not Modified";
    else if (this->responseCode == 307)
        return "307 Temporary Redirect";
    else if (this->responseCode == 308)
        return "308 Permanent Redirect";
    else if (this->responseCode == 400)
        return "400 Bad Request";
    else if (this->responseCode == 403)
        return "403 Forbidden";
    else if (this->responseCode == 404)
        return "404 Not Found";
    else if (this->responseCode == 405)
        return "405 Method Not Allowed";
    else if (this->responseCode == 413)
        return "413 Request Entity Too Large";
    else if (this->responseCode == 500)
        return "500 Internal Server Error";
    else
        return "200 OK";
}


void Response::setMethodNotAllowed(bool _tf)
{
    this->methodNotAllowed = _tf;
}

bool Response::getMethodNotAllowed(void) const
{
    return this->methodNotAllowed;
}

std::string Response::getCgiPath(void) const
{
    return cgiPath;
}

void Response::setCgiPath(std::string _cgiPath)
{
    this->cgiPath = _cgiPath;
}

std::string Response::getCgiExtension(void) const
{
    return cgiExtension;
}

void Response::setCgiExtension(std::string _cgiExtension)
{
    this->cgiExtension = _cgiExtension;
}

std::string Response::getPureLink(void) const
{
    return pureLink;
}

void Response::setPureLink(std::string _pureLink)
{
    this->pureLink = _pureLink;
}

void Response::setAutoIndex(bool _tf)
{
    this->autoIndex = _tf;
}

bool Response::getAutoIndex(void) const
{
    return this->autoIndex;
}

const std::vector<std::string> &Response::getMethods(void) const
{
    return this->_methods;
}

void Response::setMethods(std::vector<std::string> _methods)
{
    this->_methods = _methods;
}