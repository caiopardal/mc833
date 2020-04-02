#include "server.h"

char *time_path;
FILE *time_output;

int main(int argc, char *argv[])
{
  int sockfd, new_fd, pid; // listen on sock_fd, new connection on new_fd
  struct sockaddr_in server, client;
  int sin_size;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("gateway socket");
    exit(1);
  }

  if (argc == 2)
  {
    time_path = argv[1];
    time_output = fopen(time_path, "w");
  }

  memset(&server, 0, sizeof server);
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
  {
    perror("server: bind");
    exit(1);
  }

  if (listen(sockfd, BACKLOG) == -1)
  {
    perror("listen");
    exit(1);
  }

  printf("server: waiting for connections...\n");
  while (1)
  { // main accept() loop
    sin_size = sizeof client;
    new_fd = accept(sockfd, (struct sockaddr *)&client, &sin_size);
    if (new_fd == -1)
    {
      perror("accept");
      exit(1);
    }

    printf("server: got connection\n");
    if (!fork())
    {                          // this is the child process
      close(sockfd);           // child doesn't need the listener
      request_options(new_fd); // Communication function
      close(new_fd);
      exit(0);
    }
    close(new_fd); // parent doesn't need this
  }

  return 0;
}