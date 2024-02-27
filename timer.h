#pragma once

#include <sys/time.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>     // unix-like system calls read and write
#include <fcntl.h>      // unix-like file handling : open
#include <stdlib.h>     // standard C lib input output basic functions compatible with Windows
#include <stdio.h>      // standard C lib input output basic functions compatible with Windows
#include <stdbool.h>
#include <string.h>     // also from standard C lib : basic string functions like strlen
#include <pthread.h>

void startTimer();
long endTimer();
