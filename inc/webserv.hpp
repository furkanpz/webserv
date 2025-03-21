#pragma once

#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <sstream>
#include "Response.hpp"

#define OK 200
#define NOTFOUND 404
#define MAX_INT 2147483647

#define GET -2147483647
#define POST -2147483646
#define DELETE -2147483645