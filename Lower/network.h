#ifndef NETWORK_H
#define NETWORK_H

#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>

int setsocket(const char []);

#endif