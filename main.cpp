#include "webserv.hpp"
#include "WebServer.hpp"

int main(int ac, char **av)
{
    if (ac == 1)
    {
        (void)av; // wall werror wextra sussun diye koydum şimdilik
        try {
            WebServer serv("127.0.0.1", 3131);
            serv.start();
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


