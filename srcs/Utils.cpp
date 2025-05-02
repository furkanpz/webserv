
#include "webserv.hpp"
#include "Utils.hpp"

std::string Utils::intToString(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

std::string Utils::get_host_header(const std::string& request) {
    size_t host_start = request.find("Host:");
    if (host_start == std::string::npos) return "";
    size_t host_end = request.find("\r\n", host_start);
    std::string host = request.substr(host_start + 5, host_end - host_start - 5);
    size_t port_pos = host.find(":");
    if (port_pos != std::string::npos) {
        host = host.substr(0, port_pos);
    }
    return Utils::Spacetrim(host);
}

void Utils::getServerByHost(const std::string& host, Clients& client)
{
    std::vector<Server>& possible = client.servers.posibleServers;

    for (size_t i = 0;i < possible.size(); i++){
        Server& srv = possible[i];
        
        for (size_t j = 0; j < srv.server_names.size(); j++)
        {
            if (srv.server_names[j] == host)
            {
                client.server = srv;
                client.maxBodySize = srv.client_max_body_size;
                return ;
            }
        }
    }
    client.maxBodySize = client.servers.Default.client_max_body_size;
}



std::string Utils::returnErrorPages(Response &response, int ErrorType, Clients &Client)
{
    Server                                  &server = Client.server;
    std::string                             file = "";
    std::map<int, std::string>::iterator    it = server.error_pages.find(ErrorType);

    response.setResponseCode(ErrorType);
    if (it != server.error_pages.end())
        file = "." + it->second;

    std::ifstream nf(file.c_str());
    std::stringstream buffer;
    if (nf) {
        buffer << nf.rdbuf();
        return buffer.str();
    }
    return response.getResponseCodestr();
}
std::string Utils::Spacetrim(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isspace(*it))
        it++;

    std::string::const_reverse_iterator rit = s.rbegin();
    while (rit.base() != it && std::isspace(*rit))
        rit++;

    return std::string(it, rit.base());
}

std::string Utils::returnResponseHeader(Clients &client) {
    std::string header = "HTTP/1.1 ";
    int responseCode = client.response.getResponseCode();
    header += client.response.getResponseCodestr() + "\r\n";
    header += "Content-Type: text/html\r\n";
    if (responseCode == NOTALLOWED)
    {
        header += "Allow: ";
        std::vector<std::string> methods = client.response.getMethods();
        for (size_t i = 0; i < methods.size(); i++)
        {
            header += methods[i];
            if (i != methods.size() - 1)
                header += ", ";
        }
        header += "\r\n";
    }
    if (responseCode == MOVEDPERMANENTLY)
        header += "Location: " + client.response.getPureLink() + "/\r\n";
    else if (responseCode == FOUND)
        header += "Location: " + client.response.getRedirect() + "/\r\n";  
    if (!client.response.getAddHeader().empty() && responseCode == 200)
    {
        const std::string &add_header = client.response.getAddHeader();
        if (add_header.find("attachment") != std::string::npos)
        {
            std::string file_name = client.response.getFile();
            size_t i = 0;
            size_t j = file_name.find("/"); 
            while (j != std::string::npos)
            {   
                i = j;
                j = file_name.find("/", j + 1);
            }
            header += add_header + "; filename=\"" + file_name.substr(i + 1) + "\" \r\n";
        }
        else 
            header += add_header + "\r\n";
        
    }
    header += "Connection: keep-alive\r\n";
    std::ostringstream oss;
    oss << client.response.getContent().length();
    header += "Content-Length: " + oss.str() + "\r\n";
    header += "\r\n";
    header += client.response.getContent();
    return header;
}

bool readFileToString(const std::string& path, std::string& out) {
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file)
        return false;
    file.seekg(0, std::ios::end);
    std::streamoff size = file.tellg();
    if (size > 0) {
        out.resize(static_cast<std::size_t>(size));
        file.seekg(0, std::ios::beg);
        file.read(&out[0], size);
    } else {
        out.clear();
    }
    return true;
}

std::string Utils::readFile(const std::string& fileName,
                     Response& response,
                     Clients& client,
                     int code)
{
    int respCode = response.getResponseCode();
    if (respCode != 0 && respCode != 200 && respCode != -1)
        return returnErrorPages(response, respCode, client);

    struct stat st;
    if (stat(fileName.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
        const std::string& pureLink = response.getPureLink();
        if (pureLink.empty() || pureLink[pureLink.size() - 1] != '/')
            return returnErrorPages(response, MOVEDPERMANENTLY, client);

        std::string indexPath = fileName + "/index.html";
        if (access(indexPath.c_str(), R_OK) == 0) {
            std::string body;
            if (readFileToString(indexPath, body)) {
                response.setResponseCode(code);
                return body;
            }
        }
        if (response.getAutoIndex()) {
            return generateAutoIndex(fileName, client.response.getFile(), client);
        }
        return returnErrorPages(response, FORBIDDEN, client);
    }
    const Server& srv = client.server;

    if (!client.response.getCgiPath().empty() && !srv.cgi_extensioninserver.empty()) {
        if (access(srv.cgi_pathinserver.c_str(), X_OK) != 0
         || access(fileName.c_str(), F_OK) != 0
         || access(fileName.c_str(), R_OK) != 0)
        {
            if (access(fileName.c_str(), R_OK) != 0)
                return returnErrorPages(response, INTERNALSERVERERROR, client);
            return returnErrorPages(response,
                    access(srv.cgi_pathinserver.c_str(), X_OK) != 0
                    ? INTERNALSERVERERROR : NOTFOUND,
                    client);
        }
        if (access(fileName.c_str(), X_OK) != 0)
            return returnErrorPages(response, INTERNALSERVERERROR, client);

        if (fileName.find(srv.cgi_extensioninserver) == std::string::npos)
            return returnErrorPages(response, INTERNALSERVERERROR, client);
        response.setisCGI(true);
        return "";
    }
    std::string body;
    if (readFileToString(fileName, body)) {
        response.setResponseCode(code);
        return body;
    }
    return returnErrorPages(response, NOTFOUND, client);
}


bool Utils::wait_with_timeout(pid_t pid, int timeout_seconds) {
    time_t start = std::time(NULL);
    
    while (true) {
        pid_t result = waitpid(pid, NULL, WNOHANG);
        if (result == pid)
            return true;
        else if (result == -1)
            return false;
        if (std::time(NULL) - start >= timeout_seconds)
            break;
    }

    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
    return false;
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

std::string chunkedgetline(std::istringstream &stream, int type)
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
    client.response.setFormData(result);
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
                client.response.setFormData(temp);
            else
                client.formData.append(temp);
        }
    }
}

void Utils::parseChunked(Clients &client, std::string &Body)
{
    std::istringstream tempBody(Body);
    std::string result;
    std::string line;
    int size = 0;
    std::string temp;
    
    line = chunkedgetline(tempBody, 0);
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
        while (temp.length() != (size_t)size)
        {
            line = chunkedgetline(tempBody, size);
            temp.append(line);
        }
        result.append(temp);
        line = chunkedgetline(tempBody, 2);
        line = chunkedgetline(tempBody, 0);
    }
    
    if (size == -1)
    {
        if (result.length() > client.maxBodySize)
        {
            client.response.setContent(returnErrorPages(client.response, ENTITYTOOLARGE, client));
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
    if (Body.find("0\r\n\r\n") == std::string::npos) {
        client.formData.append(Body);
        client.Events = WAIT_FORM;
    }
    else
    {
        client.response.setFormData(Body);
        client.response.setIsChunked(false);
        client.response.setContentLength(Body.length());
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
                client.response.setFormData(temp);
            else
                client.formData.append(temp);
        }
    }
    if (client.response.getFormData().length() != client.response.getContentLength())
        client.Events = WAIT_FORM;
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
            client.response.setFormData(buffer.substr(buffer.find("\r\n\r\n") + 4));
    }
    else if (contentType.find("text/plain") != std::string::npos) {
        size_t pos = buffer.find("\r\n\r\n");
        if (pos != std::string::npos)
            client.response.setFormData(buffer.substr(pos + 4));
    }
    else {
        client.response.setResponseCode(UNSUPPORTED_MEDIA_TYPE);
        client.response.setContent(returnErrorPages(client.response, UNSUPPORTED_MEDIA_TYPE, client));
    }
}

void Utils::parseContent(std::string &buffer, Clients &client)
{
    std::string request(buffer);
    Response &response = client.response;
    if (client.Events == REQUEST && client.response.getRequestType() == NONE)
    {
        if (request.find("DELETE ") == 0)
            response.setRequestType(DELETE);
        else if(request.find("POST ") == 0)
            response.setRequestType(POST);
        else if (request.find("GET ") == 0)
            response.setRequestType(GET);
        else
            response.setRequestType(NONE);
        response.setFile(getFileName(request), client.server);
        response.setcontentType(get_content_type(request));
        response.setContentLength(getContentLenght(request, response));
        if (response.getContentLength() > client.maxBodySize)
        {
            response.setContent(returnErrorPages(response, ENTITYTOOLARGE, client));
            return ;
        }
        response.setContent(readFile(response.getFile(), response, client));
        if ((response.getRequestType() == POST || response.getRequestType() == DELETE))
            getBufferFormData(buffer, client);
    }
}


bool Utils::isDirectory(const std::string& path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        return S_ISDIR(s.st_mode);
    }
    return false;
}

std::string Utils::getFileName(std::string request)
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

std::string Utils::getTime()
{
	std::time_t t = std::time(NULL);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
	return (std::string(buffer));
}

void Utils::print_response(Clients &client)
{
    Response &response = client.response;
    if (response.getRequestType() == NONE)
        return ;
    std::string meth(methods[MAX_INT - response.getRequestType()]);
    std::string servername = (client.ServerName.empty() != true) ? client.ServerName : "0.0.0.0";

    std::cout << "\033[0;34m";
    std::cout << "[INFO] " << Utils::getTime() << ": Server=" << servername << " Method=<" << meth << "> URL=\""
            << response.getPureLink() << "\" Response=" << std::setw(2) << response.getResponseCode() << std::endl;
    std::cout << "\033[0m";
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




std::string Utils::generateAutoIndex(const std::string& path, const std::string& requestPath, Clients &client) {
    DIR* dir;
    struct dirent* entry;
    struct stat info;
    std::ostringstream html;

    html << "<html><head><title>Index of " << requestPath << "</title></head>\n";
    html << "<body><h1>Index of " << requestPath << "</h1><ul>\n";

    dir = opendir(path.c_str());
    if (!dir) {
        html << "<p>Failed to open directory</p></body></html>";
        return html.str();
    }

    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;

        if (name == "." || name == "..")
            continue;

        std::string fullPath = path + "/" + name;

        if (stat(fullPath.c_str(), &info) == 0 && S_ISDIR(info.st_mode)) {
            name += "/";
        }

        html << "<li><a href=\"" << client.response.getPureLink() +  name << "\">" << name << "</a></li>\n";
    }

    closedir(dir);

    html << "</ul></body></html>";
    return html.str();
}

void Utils::printInfo(const std::string &info) {
    std::cout << "\033[0;32m" << "[INFO] " << Utils::getTime() << ": " << info << "\033[0m" << std::endl;
}
