#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define SERVER_PORT 6767
#define BUFFER_LENGTH 256
#define NAME_LENGTH 50

void * readServer(void* arg);

int main(int argc, char* argv[])
{
  struct sockaddr_in serverAddr = {AF_INET, htons(SERVER_PORT)};
  char buffer[BUFFER_LENGTH];
  char username[NAME_LENGTH];
  struct hostent* host;
  int sock, quit = 0;
  pthread_t readThread;
  
  if(argc != 2)
  {
    printf("Incorrect arguments\n");
    printf("%s Hostname\n", argv[0]);
    exit(1);
  }
  
  host = gethostbyname(argv[1]);
  
  if(host == NULL)
  {
    printf("Unknown Host: %s\n",argv[1]);
    exit(1);
  }
  
  bcopy(host->h_addr_list[0], (char*)&serverAddr.sin_addr, host->h_length);
  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  if(sock == -1)
  {
    perror("Error! Could not set up Socket!");
    exit(1);
  }
  
  if(connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
  {
    perror("Error! Failed to Connect to Server!");
    exit(1);
  }
  
  pthread_create(&readThread, NULL, &readServer, &sock);
  
  printf("Connected to %s\n", argv[1]);
  
  printf("Username: ");
  
  do
  {
    fgets(username, NAME_LENGTH, stdin);
  }while(username[0] == '\n');
  
  strtok(username, "\n");
  write(sock, username, NAME_LENGTH);
  
  printf("Welcome to the Chat Room, %s!\n", username);
  
  while(quit == 0 && fgets(&buffer, BUFFER_LENGTH, stdin) != EOF)
  {
    if(strcmp(buffer, "/exit\n") == 0)
      quit = 1;
    else
      write(sock, buffer, BUFFER_LENGTH);
  }
  
  
  close(sock);
  
  return 0;
}

void * readServer(void* arg)
{
  int* sock = arg;
  char message[BUFFER_LENGTH+NAME_LENGTH+3];
  
  while(read(*sock, message, BUFFER_LENGTH+NAME_LENGTH+3) != 0)
  {
    printf("%s", message);
    message[0] = '\0';
  }
    
  return;
}