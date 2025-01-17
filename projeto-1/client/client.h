#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFLEN 2048 // Length of the message buffer
#define PORT "3490"  // the port client will be connecting to

// Funcions signatures
void make_request(int);
void receive_data(int, char *);
char *get_path(char *);

// Debug wrapper for send
int write_d(int socket, char *buffer, int length)
{
  int i, r_val;

  // Fiil message to standard size of buffer
  for (i = length; i < BUFFLEN; ++i)
    buffer[i] = '\0';

  if ((r_val = send(socket, buffer, BUFFLEN, 0)) == -1)
  {
    perror("ERROR: send");
    exit(1);
  }
  else if (r_val == 0)
  {
    printf("ERROR: pairing socket is closed\n");
    exit(1);
  }

  return r_val;
}

// Debug wrapper for recv
int read_d(int socket, char *buffer)
{
  int r_val, total = 0;

  while (total != BUFFLEN)
  {
    if ((r_val = recv(socket, &buffer[total], (BUFFLEN - total), 0)) == -1)
    {
      perror("ERROR: send");
      exit(1);
    }
    else if (r_val == 0)
    { // if client not responding
      printf("ERROR: pairing socket is closed\n");
      exit(1);
    }
    else
    {
      total += r_val;
    }
  }

  return total;
}
