#include "client.h"

char prot;
struct sockaddr *servaddr;
int len;

int main(int argc, char *argv[])
{
  int sock_udp, rv;
  struct addrinfo hints, *p, *servers;
  struct timeval timeout;

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

  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  setsockopt(sock_udp, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

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
  int socket;

  socket = sock_udp;

  printf("connection is set...\n");

  const char *helpInstructions =
      "Opções disponíveis:\n"
      "1 - Cadastrar um novo filme recebendo como resposta positiva o seu respectivo identificador;\n"
      "2 - Remover um filme a partir do seu identificador;\n"
      "3 - Listar o título e salas de exibição de todos os filmes;\n"
      "4 - Listar todos os títulos de filmes de um determinado gênero;\n"
      "5 - Dado o identificador de um filme, retornar o título do filme;\n"
      "6 - Dado o identificador de um filme, retornar todas as informações deste filme;\n"
      "7 - Listar todas as informações de todos os filmes;\n"
      "h - Listar todas as opções de requisições disponíveis;\n";

  printf("%s\n", helpInstructions);

  while (1)
  {
    int interceptor = 0;

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
      interceptor = receive_data(socket, buffer);

      if (interceptor < 0)
        printf("\nError: Movie was added, but the server couldn't retrive it's identifier. So it's identifier is the name that you typed!\n");

      break;
    case '2':
      if (strtok(NULL, " ") == NULL)
      {
        printf("No movie identifier provided! Closing the connection...\n");
        exit(1);
      }

      printf("removing movie...\n\n");
      printf("movie removed\n");
      break;
    case '3':
      printf("awaiting titles and movie rooms...\n\n");
      while (buffer[0])
      {
        interceptor = receive_data(socket, buffer);
        if (interceptor < 0)
          break;
      }

      if (interceptor < 0)
        printf("Error: Couldn't receive all movies\n");
      else
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
      {
        interceptor = receive_data(socket, buffer);
        if (interceptor < 0)
          break;
      }

      if (interceptor < 0)
        printf("Error: Couldn't receive all the movies by genre\n");
      else
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
      {
        interceptor = receive_data(socket, buffer);
        if (interceptor < 0)
          break;
      }

      if (interceptor < 0)
        printf("Error: Movie Title could not be received\n");
      else
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
      {
        int interceptor = receive_data(socket, buffer);
        if (interceptor < 0)
          break;
      }

      if (interceptor < 0)
        printf("Error: Movie info could not be receiveid\n");
      else
        printf("movie info received\n");
      break;
    case '7':
      printf("awaiting all movies...\n\n");
      if (write_udp(socket, buffer, strlen(buffer), servaddr) < 0)
        return;
      while (buffer[0])
      {
        interceptor = receive_data(socket, buffer);
        if (interceptor < 0)
          break;

        interceptor = read_udp(socket, buffer, servaddr, &len);
        if (interceptor < 0)
          break;
      }

      if (interceptor < 0)
        printf("\nError: Movies infos could not be received\n");
      else
        printf("\nall movies received\n");
      break;
    case 'h':
      while (buffer[0])
      {
        interceptor = receive_data(socket, buffer);
        if (interceptor < 0)
          break;
      }
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
int receive_data(int socket, char *buffer)
{
  buffer[0] = 'x';
  while (buffer[0] != '\0')
  { // print all messages
    if (read_udp(socket, buffer, servaddr, &len) < 0)
      return -1;
    printf("%s\n", buffer);
  }

  return 0;
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