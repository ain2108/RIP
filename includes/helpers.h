#ifndef _HELPERS_
#define _HELPERS_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include <pthread.h>

#include <signal.h>
//#include <limits.h>
#define INT_MAX 666

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <netdb.h>
#include <arpa/inet.h>

#define SMALL_BUFFER 256
#define MAX_NODES 128
#define IP_LENGTH 16

char * getMyIP(char * buffer);
void die(char * message);
int charToChar(char * dest, char * src, int numberOfBytes);

#endif
