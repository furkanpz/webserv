#include "webserv.hpp"
#include "Utils.hpp"

std::string Utils::intToString(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string returnNotFound(Response &response)
{
    response.setResponseCode(NOTFOUND);
    std::ifstream nf(std::string("www/notFound.html").c_str());
    std::stringstream buffer;
    if (nf) {
        buffer << nf.rdbuf();
        return buffer.str();
    }
    return "";
}

std::string Utils::returnResponseHeader(Clients &client) {
    std::string header = "HTTP/1.1 ";
    header += client.response.getResponseCodestr() + "\r\n";
    header += "Content-Type: text/html\r\n";
    header += "Connection: keep-alive\r\n";
    header += "Content-Length: " + Utils::intToString(client.response.getContent().length()) + "\r\n";
    header += "\r\n";
    header += client.response.getContent();
    return header;
}

std::string Utils::readFile(const std::string &fileName, Response &response, int code)
{
    if (isDirectory(fileName))
    {
        std::string indexPath = fileName + "/index.html";
        if (access(indexPath.c_str(), R_OK) == 0)
        {
            std::ifstream indexFile(indexPath.c_str());
            if (indexFile)
            {
                std::stringstream buffer;
                buffer << indexFile.rdbuf();
                response.setResponseCode(code);
                return buffer.str();
            }
            else
                return returnNotFound(response);
        }
        else
            return returnNotFound(response);

    }
    else if (fileName.find("cgi-bin") != std::string::npos)
    {
        std::string pureFile = fileName.substr(4); // "www/" ün uzunluğu 4 olmasından dolayı 4 yazıldı sonrasında ana klasör ismine göre pureFile Alınacak!
        pureFile = pureFile.substr(0, pureFile.find("/"));
        if (pureFile != "cgi-bin")
            return "";
        if (access(fileName.c_str(), F_OK) == 0)
        {
            response.setisCGI(true);
            response.setResponseCode(code);
            return "";
        }
        else
            return returnNotFound(response);
    }
    else
    {
        std::ifstream file(fileName.c_str());
        if (file)
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            response.setResponseCode(code);
            return buffer.str();
        }
        else
            return returnNotFound(response);
    }
}


std::string get_content_type(const std::string& http_buffer) {
    std::istringstream stream(http_buffer);
    std::string line;
    
    while (std::getline(stream, line)) {
        size_t pos = line.find("Content-Type:");
        if (pos != std::string::npos) {
            return line.substr(pos + 14);
        }
    }
    return "";
}

size_t Utils::getContentLenght(std::string request, Response &response)
{
    size_t  contentLength = 0;

    size_t pos = request.find("Content-Length:");
    if (pos != std::string::npos) {
        std::istringstream iss(request.substr(pos));
        std::string temp;
        iss >> temp >> contentLength;
    }
    if (request.find("Transfer-Encoding: chunked") != std::string::npos)
    {
        response.setIsChunked(true);
        contentLength = 0;
    }
    return (contentLength);
}

int Utils::countSeperator(const std::string &buffer, const std::string &target) {
    int count = 0;
    size_t pos = buffer.find(target);

    while (pos != std::string::npos) { 
        count++;
        pos = buffer.find(target, pos + target.length()); 
    }
    return count;
}

std::string chunkedgetline(std::string& ver, std::istringstream &stream, int type)
{
   std::string temp;

   char *buffer = new char[100240];
   if (type == 0)
   {
       while (temp.find("\r\n") == std::string::npos && !stream.eof())
       {
           stream.read(buffer, 1);
           temp.append(buffer, 1);
        }
   }
   else
    {
        stream.read(buffer, type);
        temp.append(buffer, type);
    }
   delete[] buffer;
   return (temp);
}

void Utils::ChunkedCompleted(Clients &client, std::string &result)
{
    std::string key = client.response.getcontentType();
    size_t firstPos = key.find("=");

    client.formData = result;
    client.response.setContentLength(result.length());
    client.response.setContentTypeForPost(result);
    client.response.setIsChunked(false);
    if (firstPos == std::string::npos)
        return ;

    std::string seperator = key.substr(firstPos + 1);
    if (countSeperator(result, seperator) > 1)
    {
        size_t firstIndex = result.find(seperator, result.find(seperator) + 1);
        if (firstIndex != std::string::npos)
        {
            std::string temp = result.substr(firstIndex - 2);
            if (temp.length() == client.response.getContentLength())
                client.response.setContentTypeForPost(temp);
            else
                client.formData.append(temp);
        }
    }
}

void Utils::parseChunked(Clients &client, std::string &Body, int Type)
{
    std::istringstream tempBody(Body);
    std::string result;
    std::string line;
    size_t size = 0;
    std::string temp;
    
    int i = 0;
    line = chunkedgetline(Body, tempBody, 0);
    while (!line.empty())
    {
        size = 0;
        std::istringstream hexStream(line);
        hexStream >> std::hex >> size;
        if (size == 0)
        {
            size = -1;
            break;
        }
        std::string temp;
        while (temp.length() != size)
        {
            line = chunkedgetline(Body, tempBody, size);
            temp.append(line);
        }
        result.append(temp);
        line = chunkedgetline(Body, tempBody, 2);
        line = chunkedgetline(Body, tempBody, 0);
    }
    
    if (size == -1)
    {
        if (result.length() > client.maxBodySize)
        {
            client.response.setResponseCode(413);
            client.response.setContent("413 Request Entity Too Large");
            return ;
        }
        ChunkedCompleted(client, result);
    }
}
void Utils::parseChunked_FT(Clients &client, std::string &Body, int Type) {

    if (!Type) {
        size_t headerEnd = Body.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
            Body = Body.substr(headerEnd + 4);
    }
    client.formData.append(Body);
    if (client.formData.find("0\r\n\r\n") == std::string::npos) {
        client.events = WAIT_FORM;
    }
}


void Utils::doubleSeperator(std::string key, std::string &buffer, Clients &client)
{
    std::string target = "=";
    size_t firstPos = key.find(target);
    if (firstPos == std::string::npos)
        return ;
    std::string seperator = key.substr(firstPos + 1);
    if (countSeperator(buffer, seperator) > 1)
    {
        size_t firstIndex = buffer.find(seperator, buffer.find(seperator) + 1);
        if (firstIndex != std::string::npos)
        {
            std::string temp = buffer.substr(firstIndex - 2);
            if (temp.length() == client.response.getContentLength())
                client.response.setContentTypeForPost(temp);
            else
                client.formData.append(temp);
        }
    }
    if (client.response.getContentTypeForPost().length() != client.response.getContentLength()) // DEBUG İÇİN
        client.events = WAIT_FORM;
}

void Utils::getBufferFormData(std::string &buffer, Clients &client)
{
    std::string contentType = client.response.getcontentType().substr(0, client.response.getcontentType().find(";"));
    if (client.response.getIsChunked())
        parseChunked_FT(client, buffer, 0);
    else if (!contentType.find("multipart/form-data"))
        Utils::doubleSeperator(client.response.getcontentType() , buffer, client);
    else if (!contentType.find("application/x-www-form-urlencoded"))
    {
        if (buffer.find("\r\n\r\n") != std::string::npos)
            client.response.setContentTypeForPost(buffer.substr(buffer.find("\r\n\r\n") + 4));
    }
}

void Utils::parseContent(std::string &buffer, Clients &client)
{
    std::string request(buffer);
    Response &response = client.response;
    
    if (client.events == REQUEST && client.response.getRequestType() == NONE)
    {
        response.setFile(getFileName(request, client), client.server);
        response.setcontentType(get_content_type(request));
        response.setContentLength(getContentLenght(request, response));
        if (response.getContentLength() > client.maxBodySize)
        {
            response.setResponseCode(413);
            response.setContent("413 Request Entity Too Large");
            return ;
        }
        response.setContent(readFile(response.getFile(), response));
        
        if (request.find("DELETE ") == 0)
        {
            response.setRequestType(DELETE);
            getBufferFormData(buffer, client);
        }
        else if(request.find("POST ") == 0)
        {
            response.setRequestType(POST);
            getBufferFormData(buffer, client);
        } 
        else if (request.find("GET ") == 0)
            response.setRequestType(GET);
        else
            response.setRequestType(NONE);
    }
}


bool Utils::isDirectory(const std::string& path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        return S_ISDIR(s.st_mode);
    }
    return false;
}

std::string Utils::getFileName(std::string request, Clients &client)
{

    size_t pos = request.find(" ");
    if (pos == std::string::npos) {
        return "";
    }

    size_t start = pos + 1;
    size_t end = request.find(" ", start);
    if (end == std::string::npos) {
        return "";
    }

    std::string path = request.substr(start, end - start);
    if (path == "/")
        return path;
    
    if (isDirectory(path))
        return path;
    return  path;
}

void Utils::print_response(Response &response)
{
    if (response.getRequestType() == NONE)
        return ;
    std::string meth("[" + methods[MAX_INT - response.getRequestType()] + "]");
    std::cout << meth << std::setw(3)
               << " " << response.getFile() << " " << response.getResponseCode() << std::endl;
}

std::vector<std::string> Utils::split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    
    while (std::getline(tokenStream, token, delimiter)) {
        if (token.empty())
            continue;
        tokens.push_back(token);
    }
    
    return tokens;
}