#include "webserv.hpp"
#include "WebServer.hpp"

WebServer *g_server = NULL;
std::vector<Servers> g_servers;

void ServerKill(int sig) {
    (void) sig;
    delete g_server;
    std::cout << "\nServer Killed!" << std::endl;
    exit(1);
}

int main(int ac, char **av)
{
    if (ac == 2)
    {
        try {
            std::vector<Server> PureServers = parse_config(av[1]);
            g_servers = SetServers(PureServers);
            if (g_servers.size() == 0)
            {
                std::cerr << "No server found in the configuration file." << std::endl;
                return (1);
            }
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


