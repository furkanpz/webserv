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
#include <iomanip>
#include <dirent.h>
#include "config.hpp"

#include <sys/wait.h>
#include <algorithm>

static const std::string methods[3] = {"GET", "POST", "DELETE"};

#define MAX_EVENTS 10 


#define SEEOF 0
#define OK 200
#define MOVEDPERMANENTLY 301
#define FOUND 302
#define NOTMODIFIED 304
#define TEMPORARYREDIRECT 307
#define PERMANENTREDIRECT 308
#define BADREQUEST 400
#define FORBIDDEN 403
#define NOTFOUND 404
#define NOTALLOWED 405
#define NOTACCEPTABLE 406
#define TIMEOUT 408
#define LENGTHREQUIRED 411
#define ENTITYTOOLARGE 413
#define UNSUPPORTED_MEDIA_TYPE 415
#define INTERNALSERVERERROR 500

#define MAX_INT 2147483647

#define GET 2147483647
#define POST 2147483646
#define DELETE 2147483645

#define NONE 0
#define REQUEST 1
#define WAIT_FORM 2
#define RECVERROR 3

#define HOME_DIR "www/"
