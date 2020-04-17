#include "client.h"

clock_t start;
char *time_path;
FILE *time_output;

int main(int argc, char *argv[])
{
  int sockfd, rv;
  struct addrinfo hints, *p, *servers;
  if (argc < 2)
  {
    fprintf(stderr, "Error: you need to pass a client hostname\n");
    exit(1);
  }

  if (argc == 3)
  {
    time_path = argv[2];
    time_output = fopen(time_path, "w");
  }

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servers)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and connect to the first we can
  for (p = servers; p != NULL; p = p->ai_next)
  {
    sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockfd == -1)
    {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      perror("client: connect");
      continue;
    }

    break;
  }

  if (p == NULL)
  {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  printf("client: connecting...\n");
  freeaddrinfo(servers); // all done with this structure
  make_request(sockfd);
  close(sockfd);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void make_request(int socket)
{
  char buffer[BUFFLEN];
  int i;

  // receive server connection set confirmation
  read_d(socket, buffer);
  printf("%s\n", buffer);

  // receive help
  read_d(socket, buffer);

  while (1)
  {
    // Scan and send user request
    printf("awaiting input:\n");
    scanf(" %[^\n]", buffer);
    if (!strlen(buffer))
      exit(1);

    write_d(socket, buffer, strlen(buffer));

    // Await server commands
    switch (strtok(buffer, " ")[0])
    {
    case '1':
      printf("adding a new movie...\n");
      read_d(socket, buffer);
      printf("movie added\n");
      // printf("new movie id: %s\n", movieId);
      break;
    case '2':
      printf("removing the movie...\n");
      read_d(socket, buffer);
      printf("movie removed\n");
      break;
    case '3':
      printf("awaiting titles and movie rooms...\n");
      while (buffer[0])
        receive_data(socket, buffer);
      printf("received movies\n");
      break;
    case '4':
      printf("awaiting movies by genre...\n");
      while (buffer[0])
        receive_data(socket, buffer);
      printf("movies by genre received\n");
      break;
    case '5':
      printf("awaiting \"%s\" title...\n", strtok(NULL, " "));
      while (buffer[0])
        receive_data(socket, buffer);
      printf("movie title received\n");
      break;
    case '6': // Get full movie info
      printf("awating \"%s\" info...\n", strtok(NULL, " "));
      while (buffer[0])
        receive_data(socket, buffer);
      printf("movie info received\n");
      break;
    case '7':
      printf("awaiting all movies...\n");
      read_d(socket, buffer);
      while (buffer[0])
      {
        receive_data(socket, buffer);
        read_d(socket, buffer);
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
    read_d(socket, buffer);
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
