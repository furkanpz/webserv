#pragma once

#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <iostream>
#include <vector>

static const std::string methods[3] = {"GET", "POST", "DELETE"};

#define MAX_EVENTS 10 


#define OK 200
#define NOTFOUND 404
#define BADREQUEST 400
#define FORBIDDEN 403
#define MAX_INT 2147483647

#define GET 2147483647
#define POST 2147483646
#define DELETE 2147483645

#define NONE 0
#define REQUEST 1
#define WAIT_FORM 2

#define HOME_DIR "www/"
