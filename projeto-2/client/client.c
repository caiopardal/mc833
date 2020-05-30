#include "client.h"

char prot;
struct sockaddr *servaddr;
int len;

int main(int argc, char *argv[])
{
  int sock_udp, rv;
  struct addrinfo hints, *p, *servers;

  if (argc < 2)
  {
    fprintf(stderr, "Error: you need to pass a client hostname\n");
    exit(1);
  }

  // Alocate search parameters
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;

  // Search for UDP servers
  hints.ai_socktype = SOCK_DGRAM;
  if ((rv = getaddrinfo(argv[1], UDP_PORT, &hints, &servers)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the UDP results and use the first we can
  for (p = servers; p != NULL; p = p->ai_next)
  {
    if ((sock_udp = socket(p->ai_family, p->ai_socktype, 0)) == -1)
    {
      perror("client: socket");
      continue;
    }
    else
      break;
  }

  // If no server was found within the specidications, end.
  if (p == NULL)
  {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }
  // Save destination server for UDP
  servaddr = p->ai_addr;

  // Make requests to udp server
  make_request(sock_udp, servaddr);

  freeaddrinfo(servers); // all done with this structure
  close(sock_udp);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
void make_request(int sock_udp, struct sockaddr *servaddr)
{
  char buffer[BUFFLEN];
  int i, socket;

  socket = sock_udp;

  // // receive server connection confirmation
  // read_udp(socket, buffer, servaddr, &len);
  // printf("%s\n", buffer);

  // // receive help
  // read_udp(socket, buffer, servaddr, &len);
  // printf("%s\n", buffer);

  while (1)
  {
    // Scan and send user request
    printf("awaiting input:\n");
    scanf(" %[^\n]", buffer);
    if (!strlen(buffer))
      exit(1);

    if (write_udp(socket, buffer, strlen(buffer), servaddr) < 0)
      return;

    // Await server commands
    switch (strtok(buffer, " ")[0])
    {
    case '1':
      printf("adding a new movie...\n\n");

      if (strtok(NULL, " ") == NULL)
      {
        printf("No movie info provided! Closing the connection...\n");
        exit(1);
      }

      printf("Movie Identifier: ");
      receive_data(socket, buffer);
      break;
    case '2':
      if (strtok(NULL, " ") == NULL)
      {
        printf("No movie identifier provided! Closing the connection...\n");
        exit(1);
      }

      printf("removing movie...\n\n");
      if (write_udp(socket, buffer, strlen(buffer), servaddr) < 0)
        return;
      printf("movie removed\n");
      break;
    case '3':
      printf("awaiting titles and movie rooms...\n\n");
      while (buffer[0])
        receive_data(socket, buffer);
      printf("received movies\n");
      break;
    case '4':
      if (strtok(NULL, " ") == NULL)
      {
        printf("No movie genre provided! Closing the connection...\n");
        exit(1);
      }

      printf("awaiting movies by genre...\n\n");
      while (buffer[0])
        receive_data(socket, buffer);
      printf("movies by genre received\n");
      break;
    case '5':
      if (strtok(NULL, " ") == NULL)
      {
        printf("No movie identifier provided! Closing the connection...\n");
        exit(1);
      }

      printf("awaiting movie title...\n\n");
      while (buffer[0])
        receive_data(socket, buffer);
      printf("movie title received\n");
      break;
    case '6':
      if (strtok(NULL, " ") == NULL)
      {
        printf("No movie identifier provided! Closing the connection...\n");
        exit(1);
      }

      printf("awaiting movie info...\n\n");
      while (buffer[0])
        receive_data(socket, buffer);
      printf("movie info received\n");
      break;
    case '7':
      printf("awaiting all movies...\n\n");
      if (write_udp(socket, buffer, strlen(buffer), servaddr) < 0)
        return;
      while (buffer[0])
      {
        receive_data(socket, buffer);
        read_udp(socket, buffer, servaddr, &len);
      }
      printf("\nall movies received\n");
      break;
    case 'h':
      while (buffer[0])
        receive_data(socket, buffer);
      break;
    case 'e':
      return;
    default:
      printf("invalid option\n");
    }
  }

  return;
}

/*## Functions for messages ##################################*/

// Receive messages that are going to be printed in terminal
void receive_data(int socket, char *buffer)
{

  buffer[0] = 'x';
  while (buffer[0] != '\0')
  { // print all messages
    read_udp(socket, buffer, servaddr, &len);
    printf("%s\n", buffer);
  }

  return;
}

/*## Functions for transfering files ##################################*/

// Gets the full path of the file to be sent
char *get_path(char *path)
{
  char szTmp[32];
  int bytes;

  sprintf(szTmp, "/proc/%d/exe", getpid()); // get this process origin file path
  bytes = readlink(szTmp, path, BUFFLEN);   // save full path

  for (bytes; path[bytes] != '/'; --bytes)
    ;                     // removes the process name
  path[bytes + 1] = '\0'; // add eof

  return path; // return path size and full path
}