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
    std::string line;
    int line_number = 0;

    if (!file.is_open())
	{
        std::cerr << "nginx: [emerg] could not open configuration file \"" << filename << "\": No such file or directory" << std::endl;
        return servers;
    }

    while (std::getline(file, line))
	{
        line_number++;
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
            continue;
        }
        std::string block_end = "}";
        if (line.find(block_end) != std::string::npos && in_server_block && !in_location_block)
		{
            if (current_server.port == -1)
			{
                current_server.port = 80; // NGINX varsayılanı
            }
            if (current_server.host.empty()) {
                current_server.host = "0.0.0.0"; // NGINX varsayılanı
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
            if (path_start == std::string::npos || path_end == std::string::npos)
			{
                std::cerr << "nginx: [emerg] invalid location syntax in " << filename << ":" << line_number << std::endl;
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
    		ss >> port_str;
    		port_str = port_str.substr(0, port_str.find(";"));
    		std::stringstream port_ss(port_str);
    		port_ss >> current_server.port;
    		if (port_ss.fail() || !port_ss.eof())
			{
    		    std::cerr << "Error: Geçersiz port numarası: \"" << port_str << "\" (line " << line_number << ")" << std::endl;
    		    file.close();
    		    servers.clear();
    		    return servers;
    		}
    		std::string extra;
    		if (ss >> extra)
			{
    		    std::cerr << "Error: Geçersiz argüman sayısı in \"listen\" direktifinde (line " << line_number << ")" << std::endl;
    		    file.close();
    		    servers.clear();
    		    return servers;
    		}
    		if (current_server.port <= 0 || current_server.port > 65535)
			{
    		    std::cerr << "Error: Geçersiz port numarası: \"" << current_server.port << "\" (line " << line_number << ")" << std::endl;
    		    file.close();
    		    servers.clear();
    		    return servers;
    		}
		}
        else if (key == "host")
		{
                std::string host;
                ss >> host;
                host = host.substr(0, host.find(";"));
                std::string extra;
                if (ss >> extra)
				{
                    std::cerr << "nginx: [emerg] invalid number of arguments in \"host\" directive in " << filename << ":" << line_number << std::endl;
                    file.close();
                    servers.clear();
                    return servers;
                }
                if (host.empty() || !is_valid_ip(host))
				{
                    std::cerr << "nginx: [emerg] invalid host in \"host\" directive in " << filename << ":" << line_number << std::endl;
                    file.close();
                    servers.clear();
                    return servers;
                }
                current_server.host = host;
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
                ss >> size;
                size = size.substr(0, size.find("M"));
                current_server.client_max_body_size = std::atoi(size.c_str()) * 1024 * 1024;
                if (current_server.client_max_body_size <= 0)
				{
                    std::cerr << "nginx: [emerg] invalid value \"" << size << "M\" in \"client_max_body_size\" directive in " << filename << ":" << line_number << std::endl;
                    file.close();
                    servers.clear();
                    return servers;
                }
            }
            else if (key == "error_page")
			{
                int code;
                std::string path;
                ss >> code >> path;
                if (code < 100 || code > 599)
				{
                    std::cerr << "nginx: [emerg] invalid error_page code \"" << code << "\" in " << filename << ":" << line_number << std::endl;
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
						current_server.rootLocation = serverinroot;
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
                        current_server.rootLocation = root;
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
                        std::cerr << "nginx: [emerg] invalid value \"" << value << "\" in \"autoindex\" directive in " << filename << ":" << line_number << std::endl;
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
                else if (key == "redirect")
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
                else if (key == "upload_limit")
				{
                    std::string limit;
                    ss >> limit;
                    limit = limit.substr(0, limit.find("M"));
                    current_location.upload_limit = std::atoi(limit.c_str()) * 1024 * 1024;
                    if (current_location.upload_limit <= 0)
					{
                        std::cerr << "nginx: [emerg] invalid value \"" << limit << "M\" in \"upload_limit\" directive in " << filename << ":" << line_number << std::endl;
                        file.close();
                        servers.clear();
                        return servers;
                    }
                }
                else
				{
                    std::cerr << "nginx: [emerg] unknown directive \"" << key << "\" in " << filename << ":" << line_number << std::endl;
                    file.close();
                    servers.clear();
                    return servers;
                }
            }
            else
			{
                std::cerr << "nginx: [emerg] unknown directive \"" << key << "\" in " << filename << ":" << line_number << std::endl;
                file.close();
                servers.clear();
                return servers;
            }
        }
    }
    if (in_server_block || in_location_block)
	{
        std::cerr << "nginx: [emerg] unexpected end of file, expecting \"}\" in " << filename << ":" << line_number << std::endl;
        file.close();
        servers.clear();
        return servers;
    }
    file.close();
    return servers;
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

void print_servers(const std::vector<Server>& servers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        const Server& server = servers[i];
		std::cout << "serverinroot" << server.serverinroot << "\n";
        std::cout << "Server " << i + 1 << ":\n";
        std::cout << "  Host: " << server.host << "\n";
        std::cout << "  Port: " << server.port << "\n";
        std::cout << "  Root Location: " << server.rootLocation << "\n";
        std::cout << "  Server Names: ";
        for (size_t j = 0; j < server.server_names.size(); ++j) {
            std::cout << server.server_names[j];
            if (j != server.server_names.size() - 1)
                std::cout << ", ";
        }
        std::cout << "\n";
        std::cout << "  Client Max Body Size: " << server.client_max_body_size << "\n";
        std::cout << "  Error Pages:\n";
        for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); it != server.error_pages.end(); ++it) {
            std::cout << "    " << it->first << " => " << it->second << "\n";
        }
        std::cout << "  Locations:\n";
        for (size_t j = 0; j < server.locations.size(); ++j) {
            const Location& loc = server.locations[j];
            std::cout << "    Location " << j + 1 << ":\n";
            std::cout << "      Path: " << loc.path << "\n";
            std::cout << "      Root: " << loc.root << "\n";
            std::cout << "      Methods: ";
            for (size_t k = 0; k < loc.methods.size(); ++k) {
                std::cout << loc.methods[k];
                if (k != loc.methods.size() - 1)
                    std::cout << ", ";
            }
            std::cout << "\n";
            std::cout << "      Autoindex: " << (loc.autoindex ? "on" : "off") << "\n";
            std::cout << "      Index: " << loc.index << "\n";
            std::cout << "      Redirect: " << loc.redirect << "\n";
            std::cout << "      CGI Extension: " << loc.cgi_extension << "\n";
            std::cout << "      CGI Path: " << loc.cgi_path << "\n";
            std::cout << "      Upload Root: " << loc.upload_root << "\n";
            std::cout << "      Upload Limit: " << loc.upload_limit << "\n";
        }
        std::cout << "\n";
    }
}
