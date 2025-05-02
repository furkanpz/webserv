#include "webserv.hpp"
#include "Utils.hpp"
#include "WebServer.hpp"

WebServer *g_server = NULL;
std::vector<Servers> g_servers;
std::vector<Server> PureServers;

void ServerKill(int sig) {
    (void) sig;
    delete g_server;
    PureServers.empty();
    Utils::printInfo("Servers killed");
    exit(1);
}

int main(int ac, char **av)
{
    if (ac == 2)
    {
        try {
            Utils::printInfo("Parsing configuration file.");
            PureServers = parse_config(av[1]);
            if (PureServers.size() == 0)
            {
                std::cout << "\033[31m";
                std::cout << "[ERROR] " << Utils::getTime() << ": " << "No server found in the configuration file." << std::endl;
                return (1);
            }
            g_servers = SetServers(PureServers);
            Utils::printInfo("Configuration file parsed");
            g_server = new WebServer(g_servers);
            signal(SIGINT, ServerKill);
            signal(SIGPIPE, SIG_IGN);
            g_server->start();
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            return (1);
        }
        return (0);
    }
    std::cerr << "Please try : ./webserv [configuration file]" << std::endl; 
    return (1);

}


