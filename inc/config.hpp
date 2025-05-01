#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <iostream>
#include <fstream>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

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
    std::string add_header;
    unsigned long upload_limit;
};

struct Server {
    unsigned long client_max_body_size;
    int port;
    std::string host;
    std::vector<std::string> server_names;
    std::map<int, std::string> error_pages;
    int rootLocation;
	std::string serverinroot;
	std::string cgi_extensioninserver;
    std::string cgi_pathinserver;
    std::vector<Location> locations;
    struct addrinfo first, *res;
    int addrLen;
    struct sockaddr_in address;
    int serverFd;
    std::vector<Server> possibleServers;

	Server() : client_max_body_size(1024 * 1024), port(0){}
};

struct Servers {
    Server Default;
    std::vector<Server> posibleServers;
};
bool is_valid_ip(const std::string& ip);
std::vector<Server> parse_config(const std::string& filename);
void print_servers(const std::vector<Server>& servers);
bool check_braces(const std::string& filename);
void printserversandposibleservers(std::vector<Servers> &servers);
std::vector<Servers> SetServers(std::vector<Server> &servers);
#endif