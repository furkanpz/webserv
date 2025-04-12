#include "webserv.hpp"
#include "WebServer.hpp"
#include "config.hpp"

WebServer *g_server = NULL;

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
            std::vector<Server> servers = parse_config(av[1]);
            for (std::vector<Server>::size_type i = 0; i < servers.size(); i++)
            {
                
            }
            WebServer *serv  = new WebServer("127.0.0.1", 4443);
            g_server = serv;
            signal(SIGINT, ServerKill);
            signal(SIGPIPE, SIG_IGN);
            serv->start();
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


