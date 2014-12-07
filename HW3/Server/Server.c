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
void writeAllClients(char* buffer, int length);
void signalHandler(int signal);

struct Client
{
  pthread_t thread;
  int ID;
  char userName[NAME_LENGTH];
};

pthread_mutex_t clientLock;

int clientCount = 0;
struct Client clients[MAX_CLIENTS];

int main()
{
  int sock;
  struct sockaddr_in serverAddr = {AF_INET, htons(SERVER_PORT)};
  struct sockaddr_in clientAddr = {AF_INET};
  socklen_t clientLen = sizeof(clientAddr);
  int newClient;
  int i = 0;
  
  signal(SIGINT, signalHandler);

  for(i = 0; i < MAX_CLIENTS; i++)
    clients[i].ID = 0;
  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1)
  {
    perror("SERVER: ERROR! Could not set up Socket!");
    exit(1);
  }
  
  if(bind(sock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) == -1)
  {
    perror("SERVER: ERROR! Could not bind Socket!");
    exit(1);
  }

  if(listen(sock, 1) == -1)
  {
    perror("SERVER: ERROR! Could not listen for Clients");
    exit(1);
  }

  //Socket set up, Listening.
  printf("SERVER: Server is listening for clients . . . \n");

  //While we're accepting clients...
  while((newClient = accept(sock, (struct sockaddr*) &clientAddr, &clientLen)) > 0)
  {
    if(clientCount < MAX_CLIENTS)
    {
      pthread_mutex_lock(&clientLock);
      
      i = 0;
      while( i < MAX_CLIENTS && clients[i].ID != 0)
        i++;
        
      clients[i].ID = newClient;
      clientCount++;
      pthread_create(&clients[i].thread, NULL, &readClient, &clients[i]);
      
      pthread_mutex_unlock(&clientLock);
      
      
    }
  }

  
  for(i = 0; i < MAX_CLIENTS; i++)
    if(clients[i].ID > 0);
      close(clients[i].ID);
      
  close(socket);
  unlink(serverAddr.sin_addr);
	return 0;
}

void * readClient(void* arg)
{
  struct Client *client = arg;
  char buffer[BUFFER_LENGTH];
  char returnMessage[BUFFER_LENGTH+NAME_LENGTH+3];
  char welcome[NAME_LENGTH+20];
  
  read(client->ID, client->userName, NAME_LENGTH);
  
  pthread_mutex_lock(&clientLock);
  
  strcat(welcome, client->userName);
  strcat(welcome, " has connected\n");
  printf("%s", welcome);
  writeAllClients(welcome, strlen(welcome));
  
  pthread_mutex_unlock(&clientLock);
  
  while( (read(client->ID, buffer, BUFFER_LENGTH)) != 0)
  {
    pthread_mutex_lock(&clientLock);
    
    sprintf(returnMessage, "%s: %s", client->userName, buffer);
    printf("%s", returnMessage);
    writeAllClients(returnMessage, strlen(returnMessage));
    
    pthread_mutex_unlock(&clientLock);
  }
  
  pthread_mutex_lock(&clientLock);
    
  sprintf(returnMessage, "%s %s", client->userName, "has disconnected.\n");
  printf("%s", returnMessage);
  writeAllClients(returnMessage, strlen(returnMessage));
  close(client->ID);
  client->ID = 0;
  
  pthread_mutex_unlock(&clientLock);
  
  pthread_exit(NULL);
  return;
}

void writeAllClients(char* buffer, int length)
{
  int i = 0;
  for(i = 0; i < MAX_CLIENTS; i++)
    if(clients[i].ID != 0)
      write(clients[i].ID, buffer, length+1);
  
  return;
}

void signalHandler(int signal)
{
  if(signal == SIGINT)
  {
    pthread_mutex_lock(&clientLock);
    
    printf("Server will shutdown in 10 Seconds. . .\n");
    writeAllClients("/exit", 6);
    
    pthread_mutex_unlock(&clientLock);
    
    sleep(10);
    
    exit(1);
  }
}