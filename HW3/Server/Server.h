
#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 6767
#define MAX_CLIENTS 10
#define BUFFER_LENGTH 256
#define NAME_LENGTH 50

void * readClient(void * arg);
void writeAllClients(char* buffer, int length, int caller);
void signalHandler(int signal);

struct Client
{
  pthread_t thread;
  int ID;
  char userName[NAME_LENGTH];
};

#endif