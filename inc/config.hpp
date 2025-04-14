#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>

struct Location {
    std::string path;
    std::string root;
    std::vector<std::string> methods;
    bool autoindex;
    std::string index;
    std::string redirect;
    std::string cgi_extension;
    std::string cgi_path;
    std::string upload_root;
    unsigned long upload_limit;
};

struct Server {
    int port;
    std::string host;
    std::vector<std::string> server_names;
    unsigned long client_max_body_size;
    std::map<int, std::string> error_pages;
    std::string rootLocation;
	std::string serverinroot;
	std::string cgi_extensioninserver;
    std::string cgi_pathinserver;
    std::vector<Location> locations;
	Server() : client_max_body_size(1024 * 1024), port(0){}
};
bool is_valid_ip(const std::string& ip);
std::vector<Server> parse_config(const std::string& filename);
void print_servers(const std::vector<Server>& servers);

#endif