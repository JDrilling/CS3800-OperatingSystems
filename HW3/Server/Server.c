//Authors: Jacob Drilling and Dzu Pham
//Desc: "Group Talk" Assignment. Holds a chatroom for 10 people on port 6767


#include "Server.h"

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
  
  //Setup Signal Handler
  signal(SIGINT, signalHandler);

  //Client ID == 0 means Client doesn't exist.
  for(i = 0; i < MAX_CLIENTS; i++)
    clients[i].ID = 0;
  
  /*----------- Begin Socket Setup ----------------*/
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
  /*-------------- End Socket Setup ---------------*/
  
  //Socket set up, Listening.
  printf("SERVER: Server is listening for clients . . . \n");

  //While we're accepting clients...
  while((newClient = accept(sock, (struct sockaddr*) &clientAddr, &clientLen)) > 0)
  {
    //Don't accept a Client if we have the max number
    if(clientCount < MAX_CLIENTS)
    {
      //Lock for editing client array.
      pthread_mutex_lock(&clientLock);
      
      i = 0;
      while( i < MAX_CLIENTS && clients[i].ID != 0)
        i++;
        
      clients[i].ID = newClient;
      clientCount++;
      pthread_create(&clients[i].thread, NULL, &readClient, &clients[i]);
      //New read thread created for the new client.
      
      pthread_mutex_unlock(&clientLock);
      
    }
  }

  
  
  //Close Sockets + join threads.
  for(i = 0; i < MAX_CLIENTS; i++)
    if(clients[i].ID > 0)
    {
      pthread_join(clients[i].thread, NULL);
      close(clients[i].ID);
    }
      
  close(socket);
  unlink(serverAddr.sin_addr);
	return 0;
}

//Reads messages from Client.
void * readClient(void* arg)
{
  struct Client *client = arg;
  char buffer[BUFFER_LENGTH];
  char returnMessage[BUFFER_LENGTH+NAME_LENGTH+3];
  char welcome[NAME_LENGTH+20];
  
  read(client->ID, client->userName, NAME_LENGTH);
  
  //Lock to Access clients[]
  pthread_mutex_lock(&clientLock);
  
  //Get Username
  strcat(welcome, client->userName);
  strcat(welcome, " has connected\n");
  printf("%s", welcome);
  writeAllClients(welcome, strlen(welcome), client->ID);
  
  pthread_mutex_unlock(&clientLock);
  
  //Read messages.
  while( (read(client->ID, buffer, BUFFER_LENGTH)) != 0)
  {
    pthread_mutex_lock(&clientLock);
    
    sprintf(returnMessage, "%s: %s", client->userName, buffer);
    printf("%s", returnMessage);
    writeAllClients(returnMessage, strlen(returnMessage), client->ID);
    
    pthread_mutex_unlock(&clientLock);
  }
  
  //If client exists display leave message.
  pthread_mutex_lock(&clientLock);
    
  sprintf(returnMessage, "%s %s", client->userName, "has disconnected.\n");
  printf("%s", returnMessage);
  writeAllClients(returnMessage, strlen(returnMessage), client->ID);
  close(client->ID);
  client->ID = 0;
  
  pthread_mutex_unlock(&clientLock);
  
  //exit thread.
  pthread_exit(NULL);
  return;
}


//Writes to all clients except caller.
//If caller == 0. Then it writes to everyone.
void writeAllClients(char* buffer, int length, int caller)
{
  int i = 0;
  for(i = 0; i < MAX_CLIENTS; i++)
    if(clients[i].ID != 0 && clients[i].ID != caller)
      write(clients[i].ID, buffer, length+1);
  
  return;
}


//Handle Cntrl+C
void signalHandler(int signal)
{
  if(signal == SIGINT)
  {
    pthread_mutex_lock(&clientLock);
    
    printf("Server will shutdown in 10 Seconds. . .\n");
    writeAllClients("/exit", 6, 0);
    
    int i;
    for(i = 0; i < MAX_CLIENTS; i++)
      if(clients[i].ID > 0)
        close(clients[i].ID);
      
    close(socket);
    
    pthread_mutex_unlock(&clientLock);
    
    sleep(10);
    
    exit(1);
  }
}