#include "client.h"

char *time_path;
FILE *time_output;

struct timeval t1, t2;
double elapsed;

int main(int argc, char *argv[])
{
  int sockfd, rv;
  struct addrinfo hints, *p, *servers;
  if (argc < 2)
  {
    fprintf(stderr, "usage: client hostname\n");
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