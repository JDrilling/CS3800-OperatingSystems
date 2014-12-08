
#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 6767
#define BUFFER_LENGTH 256
#define NAME_LENGTH 50

void * readServer(void* arg);
void signalHandler(int signal);

#endif