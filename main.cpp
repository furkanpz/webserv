#include "webserv.hpp"
#include "WebServer.hpp"

WebServer *g_server = NULL;

void ServerKill(int sig) {
    (void) sig;
    delete g_server;
    std::cout << "\nServer Killed!" << std::endl;
    exit(1);
}

int main(int ac, char **av)
{
    if (ac == 1)
    {
        (void)av; // wall werror wextra sussun diye koydum şimdilik
        try {
            WebServer *serv  = new WebServer("127.0.0.1", 4444);
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


