#include "webserv.hpp"
#include "WebServer.hpp"

int main(int ac, char **av)
{

    WebServer serv("127.0.0.1", 8080);

    serv.start();
}