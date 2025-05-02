#include "config.hpp"
#include "Utils.hpp"

std::vector<Server> parse_config(const std::string& filename)
{
    std::vector<Server> servers;
	Server current_server;
    Location current_location;
    bool in_server_block = false;
    bool in_location_block = false;
    std::ifstream file(filename.c_str());
    std::string tab_line;
    int line_number = 0;

    if (!file.is_open())
	{
        std::cerr << "webserv: could not open configuration file \"" << filename << "\": No such file or directory" << std::endl;
        return servers;
    }

    while (std::getline(file, tab_line))
	{
        line_number++;
		std::string line = Utils::Spacetrim(tab_line);
        if (line.empty())
            continue;
        std::string server_start = "server {";
        if (line.find(server_start) != std::string::npos)
		{
            in_server_block = true;
            current_server = Server();
            current_server.port = -1;
            current_server.host = "";
            current_server.client_max_body_size = 1 * 1024 * 1024;
            current_server.rootLocation = -1;
            continue;
        }
        std::string block_end = "}";
        if (line.find(block_end) != std::string::npos && in_server_block && !in_location_block)
		{
            if (current_server.port == -1)
			{
                current_server.port = 80;
            }
            if (current_server.host.empty()) {
                current_server.host = "0.0.0.0";
            }
            in_server_block = false;
            servers.push_back(current_server);
            continue;
        }
        std::string location_start = "location";
        if (line.find(location_start) != std::string::npos && in_server_block)
		{
            in_location_block = true;
            current_location = Location();
            std::string::size_type path_start = line.find("/");
            std::string::size_type path_end = line.find("{");

			if (path_end != std::string::npos && path_end < line.length() - 1 && line[path_end + 1] == '}')
    		{
    		    std::cerr << "webserv: unexpected \"}\" in " << filename << ":" << line_number << std::endl;
    		    file.close();
    		    servers.clear();
    		    return servers;
    		}
        	if (path_start == std::string::npos || path_end == std::string::npos)
			{
                std::cerr << "webserv: invalid location syntax in " << filename << ":" << line_number << std::endl;
                file.close();
                servers.clear();
                return servers;
            }
            std::string path = line.substr(path_start, path_end - path_start);
            current_location.path = Utils::Spacetrim(path);
            current_location.autoindex = false;
            current_location.upload_limit = 0;
            continue;
        }
        if (line.find(block_end) != std::string::npos && in_location_block)
		{
            in_location_block = false;
            current_server.locations.push_back(current_location);
            continue;
        }
        if (in_server_block)
		{
            std::stringstream ss(line);
            std::string key;
            ss >> key;
            if (key == "listen")
		    {
    	    	std::string port_str;
    	    	std::string server_str;
    	    	ss >> port_str;
    	    	port_str = port_str.substr(0, port_str.find(";"));
                server_str = port_str.substr(0, port_str.find(":"));
                if (server_str != port_str)
                {
                    if (!is_valid_ip(server_str))
		    	    {
                        std::cerr << "webserv: invalid host in \"listen\" directive in " << filename << ":" << line_number << std::endl;
                        file.close();
                        servers.clear();
                        return servers;
                    }
                    current_server.host = server_str;
                    port_str = port_str.substr(port_str.find(":") + 1);
                }
                
    	    	std::stringstream port_ss(port_str);
    	    	port_ss >> current_server.port;
    	    	if (port_ss.fail() || !port_ss.eof())
		    	{
    	    	    std::cerr << "webserv: invalid port in  \"" << port_str << "\" (line " << line_number << ")" << std::endl;
    	    	    file.close();
    	    	    servers.clear();
    	    	    return servers;
    	    	}
    	    	std::string extra;
    	    	if (ss >> extra)
		    	{
    	    	    std::cerr << "webserv: invalid number of arguments in \"listen\" directive in (line " << line_number << ")" << std::endl;
    	    	    file.close();
    	    	    servers.clear();
    	    	    return servers;
    	    	}
    	    	if (current_server.port <= 0 || current_server.port > 65535)
		    	{
    	    	    std::cerr << "webserv: invalid port number in  \"" << current_server.port << "\" (line " << line_number << ")" << std::endl;
    	    	    file.close();
    	    	    servers.clear();
    	    	    return servers;
    	    	}
		    }
            else if (key == "server_name")
		    {
                std::string name;
                while (ss >> name)
		    	{
                    if (name.find(";") != std::string::npos)
		    		{
                        name = name.substr(0, name.find(";"));
                        current_server.server_names.push_back(name);
                        break;
                    }
                    current_server.server_names.push_back(name);
                }
            }
		    else if (key == "client_max_body_size")
            {
		        std::string size;
		        ss >> std::ws;
		        std::getline(ss, size, ';');
		        size_t size_first = size.find_first_not_of(" \t");
		        size_t size_last = size.find_last_not_of(" \t");
		        if (size_first != std::string::npos && size_last != std::string::npos)
		            size = size.substr(size_first, size_last - size_first + 1);
		    	else
		            size.clear();
		        if (size.empty())
		    	{
		            std::cerr << "webserv: invalid value \"\" in \"client_max_body_size\" directive in "
		                      << filename << ":" << line_number << std::endl;
		            file.close();
		            servers.clear();
		            return servers;
		        }
		        unsigned long value = 0;
		        char* endptr = 0;
		        std::string original_size = size;
		        char last = size[size.length() - 1];
		        if (last == 'M' || last == 'm')
		    	{
		            size.erase(size.length() - 1);
		            value = std::strtoul(size.c_str(), &endptr, 10);
		            if (endptr != size.c_str() && *endptr == '\0' && value > 0)
		                value *= 1024 * 1024;
		        }
		        else if (last == 'K' || last == 'k')
		    	{
		            size.erase(size.length() - 1);
		            value = std::strtoul(size.c_str(), &endptr, 10);
		            if (endptr != size.c_str() && *endptr == '\0' && value > 0)
		                value *= 1024;
		        }
		        else if (last == 'G' || last == 'g')
		    	{
		            size.erase(size.length() - 1);
		            value = std::strtoul(size.c_str(), &endptr, 10);
		            if (endptr != size.c_str() && *endptr == '\0' && value > 0)
		                value *= 1024 * 1024 * 1024;
		        }
		        else
		            value = std::strtoul(size.c_str(), &endptr, 10);
		        if (endptr == size.c_str() || *endptr != '\0' || value == 0)
		    	{
		            std::cerr << "webserv: invalid value \"" << original_size << "\" in \"client_max_body_size\" directive in "
		                      << filename << ":" << line_number << std::endl;
		            file.close();
		            servers.clear();
		            return servers;
		        }
		        current_server.client_max_body_size = value;
		    }
            else if (key == "error_page")
		    {
                int code;
                std::string path;
                ss >> code >> path;
                if (code < 100 || code > 599)
		    	{
                    std::cerr << "webserv: invalid error_page code \"" << code << "\" in " << filename << ":" << line_number << std::endl;
                    file.close();
                    servers.clear();
                    return servers;
                }
                current_server.error_pages[code] = path.substr(0, path.find(";"));
            }
		    else if (key == "root")
		    {
		    	std::string serverinroot;
		    	ss >> serverinroot;
		    	serverinroot = serverinroot.substr(0, serverinroot.find(";"));
		    	if(in_location_block)
		    	{
		    		current_location.root = serverinroot;
		    		if (current_location.path == "/")
		    			current_server.rootLocation = current_server.locations.size();
		    	}
		    	else
		    		current_server.serverinroot = serverinroot;
		    }
            else if (in_location_block)
		    {
                if (key == "root")
		    	{
                    std::string root;
                    ss >> root;
                    root = root.substr(0, root.find(";"));
                    current_location.root = root;
                    if (current_location.path == "/")
                        current_server.rootLocation = current_server.locations.size();
                }
                else if (key == "add_header")
                {
                    std::string add_header;
                    std::string temp;
                    ss >> add_header;
                    if (add_header.find(";") == std::string::npos)
                    {
                        ss >> temp;
                        add_header.append(" " + temp);
                    }
                    add_header = add_header.substr(0, add_header.find(";"));
                    current_location.add_header = add_header;

                }
                else if (key == "methods")
		    	{
                    std::string method;
                    while (ss >> method)
		    		{
                        if (method.find(";") != std::string::npos)
		    			{
                            method = method.substr(0, method.find(";"));
                            current_location.methods.push_back(method);
                            break;
                        }
                        current_location.methods.push_back(method);
                    }
                }
                else if (key == "autoindex")
		    	{
                    std::string value;
                    ss >> value;
                    value = value.substr(0, value.find(";"));
                    if (value != "on" && value != "off")
		    		{
                        std::cerr << "webserv: invalid value \"" << value << "\" in \"autoindex\" directive in " << filename << ":" << line_number << std::endl;
                        file.close();
                        servers.clear();
                        return servers;
                    }
                    current_location.autoindex = (value == "on");
                }
                else if (key == "index")
		    	{
                    std::string index;
                    ss >> index;
                    index = index.substr(0, index.find(";"));
                    current_location.index = index;
                }
                else if (key == "return")
		    	{
                    std::string redirect;
                    ss >> redirect;
                    redirect = redirect.substr(0, redirect.find(";"));
                    current_location.redirect = redirect;
                }
                else if (key == "cgi_extension")
		    	{
                    std::string ext;
                    ss >> ext;
                    ext = ext.substr(0, ext.find(";"));
                    current_location.cgi_extension = ext;
		    		current_server.cgi_extensioninserver = ext;
                }
                else if (key == "cgi_path")
		    	{
                    std::string path;
                    ss >> path;
                    path = path.substr(0, path.find(";"));
                    current_location.cgi_path = path;
		    		current_server.cgi_pathinserver = path;
                }
                else
		    	{
                    std::cerr << "webserv: unknown directive \"" << key << "\" in " << filename << ":" << line_number << std::endl;
                    file.close();
                    servers.clear();
                    return servers;
                }
            }
            else
		    {
                std::cerr << "webserv: unknown directive \"" << key << "\" in " << filename << ":" << line_number << std::endl;
                file.close();
                servers.clear();
                return servers;
            }
        }
    }
    if (in_server_block || in_location_block)
	{
        std::cerr << "webserv: unexpected end of file, expecting \"}\" in " << filename << ":" << line_number << std::endl;
        file.close();
        servers.clear();
        return servers;
    }
    if (!check_braces(filename))
    {
        std::vector<Server>empty_servers;
        return empty_servers;
    }
    file.close();
	if (servers.empty())
    {
        std::cerr << "webserv: no \"server\" directive found in " << filename << std::endl;
        return servers;
    }

    for (size_t i = 0; i < servers.size(); i++)
    {
        for (size_t x = 0; x < servers[i].locations.size(); x++)
        {
            for (size_t j = 0; j < servers[i].locations[x].methods.size(); j++)
            {
                if (servers[i].locations[x].methods[j] != "GET" && servers[i].locations[x].methods[j] != "POST" && servers[i].locations[x].methods[j] != "DELETE")
                {
                    std::cerr << "webserv: unexpected methods" << std::endl;
                    file.close();
                    servers.clear();
                    return servers;
                }
            }
            
        }
        
    }
    return servers;
}

bool check_braces(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    std::string tab_line;
    int open_braces = 0;  
    int close_braces = 0;

    if (!file.is_open())
    {
        std::cerr << "webserv: could not open configuration file \"" << filename << "\": No such file or directory" << std::endl;
        return false;
    }

    while (std::getline(file, tab_line))
    {
        std::string line = Utils::Spacetrim(tab_line);
        if (line.empty() || line[0] == '#')
            continue;
        for (std::string::size_type i = 0; i < line.length(); ++i)
        {
            if (line[i] == '{')
                open_braces++;
            else if (line[i] == '}')
                close_braces++;
        }
    }

    file.close();

    if (open_braces != close_braces)
    {
        std::cerr << "webserv: mismatched braces in " << filename << ": " 
                  << open_braces << " opening braces, " << close_braces << " closing braces" << std::endl;
        return false;
    }

    return true;
}

bool is_valid_ip(const std::string& ip)
{
    std::istringstream iss(ip);
    std::string octet;
    int num = std::atoi(octet.c_str());
    int count = 0;

    while (std::getline(iss, octet, '.'))
    {
        if (octet.empty() || octet.size() > 3)
            return false;
        for (size_t i = 0; i < octet.size(); ++i)
        {
            if (!std::isdigit(octet[i]))
                return false;
        }
        if (octet.size() > 1 && octet[0] == '0')
            return false;
        if (num < 0 || num > 255)
            return false;
        ++count;
    }
    return count == 4;
}

std::vector<Servers> SetServers(std::vector<Server> &servers) {
    std::vector<Servers> grouped;
    while (!servers.empty()) {
        Server base = servers.front();
        Servers group;
        group.Default = base;
        for (size_t j = 1; j < servers.size(); ++j) {
            if (servers[j].port == base.port) {
                group.Default.server_names.insert(
                    group.Default.server_names.end(),
                    servers[j].server_names.begin(),
                    servers[j].server_names.end()
                );
                group.Default.client_max_body_size = std::max(
                    group.Default.client_max_body_size,
                    servers[j].client_max_body_size
                );
                group.Default.locations.insert(
                    group.Default.locations.end(),
                    servers[j].locations.begin(),
                    servers[j].locations.end()
                );
                group.Default.error_pages.insert(
                    servers[j].error_pages.begin(),
                    servers[j].error_pages.end()
                );
                group.posibleServers.push_back(servers[j]);
                servers.erase(servers.begin() + j);
                --j;
            }
        }
        group.posibleServers.insert(group.posibleServers.begin(), base);
        servers.erase(servers.begin());
        grouped.push_back(group);
    }
    return grouped;
}
