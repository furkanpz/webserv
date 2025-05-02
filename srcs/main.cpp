#include "webserv.hpp"
#include "Utils.hpp"
#include "WebServer.hpp"

WebServer *g_server = NULL;
std::vector<Servers> g_servers;
std::vector<Server> PureServers;

void ServerKill(int sig) {
    (void) sig;
    PureServers.empty();
    g_servers.empty();
    delete g_server;
    Utils::printInfo("Servers killed");
    exit(1);
}

void signalSetter()
{
    signal(SIGINT, ServerKill);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
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
                std::cout << "[ERROR] " << Utils::getTime() << ": " << "No server found in the configuration file." << "\033[0m" << std::endl;
                return (1);
            }
            g_servers = SetServers(PureServers);
            Utils::printInfo("Configuration file parsed");
            g_server = new WebServer(g_servers);
            signalSetter();
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


