//Authors: Jacob Drilling and Dzu Pham.
//Client side implementation for group chat assignment.

#include "client.h"

int quit = 0;

int main(int argc, char* argv[])
{
  struct sockaddr_in serverAddr = {AF_INET, htons(SERVER_PORT)};
  char buffer[BUFFER_LENGTH];
  char username[NAME_LENGTH];
  struct hostent* host;
  int sock;
  pthread_t readThread;
  
  //SignalHandler.
  signal(SIGINT, signalHandler);
  
  //--------------- BEGIN Socket Setup -----------------//
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
  //----------------- END Socket Setup --------------------- //

  
  printf("Connected to %s\n", argv[1]);
  
  //Get uername.
  printf("Username: ");
  
  //That Actually has letters.
  do
  {
    fgets(username, NAME_LENGTH, stdin);
  }while(username[0] == '\n');
  
  //create reader thread.
  pthread_create(&readThread, NULL, &readServer, &sock);
  
  strtok(username, "\n");
  write(sock, username, NAME_LENGTH);
  
  printf("Welcome to the Chat Room, %s!\n", username);

  //get messages from user.
  while(quit == 0 && fgets(&buffer, BUFFER_LENGTH, stdin) != EOF)
  {
    if(strcmp(buffer, "/exit\n") == 0 || strcmp(buffer, "/quit\n") == 0 || 
       strcmp(buffer, "/part\n") == 0)
      quit = 1;
    else if(strcmp(buffer, "\n") != 0)
      write(sock, buffer, strlen(buffer) + 1);

    printf("\n");
  } 
  
  //Close socket.
  close(sock);
  
  return 0;
}

//Gets messages from server.
void * readServer(void* arg)
{
  int* sock = arg;
  char message[BUFFER_LENGTH+NAME_LENGTH+3];
  
  //While we're getting messages and haven't quit...
  while(quit == 0 && read(*sock, message, BUFFER_LENGTH+NAME_LENGTH+3) != 0)
  {
    if(strcmp(message, "/exit") == 0) //if server says it's exiting.
    {
      printf("Server has shutdown. Now Exiting...\n");
      quit = 1;
      sleep(5);
      close(*sock);
      exit(1);
    }
    else    
      printf("%s\n", message);
  }
  
  pthread_exit(NULL);
  
  return;
}


//Don't allow cntrl+C
void signalHandler(int signal)
{
  if(signal == SIGINT)
    printf("Please Use /exit, /part, or /quit\n");
}
