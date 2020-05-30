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
#include <time.h>
#include <sys/time.h>

#define BUFFLEN 2048 // Length of the message buffer
#define UDP_PORT "8080"

typedef struct sockaddr *sap;

// Funcions signatures
void make_request(int, sap);
void receive_data(int, char *);
char *get_path(char *);

// UDP SEND AND RECEIVE WRAPPERS ///////////////////////////////////////////////
// Debug wrapper for sendto
int write_udp(int socket, char *buffer, int length, sap target)
{
  int i, r_val;

  // Fill message to standard size of buffer
  for (i = length; i < BUFFLEN; ++i)
    buffer[i] = '\0';

  if ((r_val = sendto(socket, (const char *)buffer, BUFFLEN, MSG_CONFIRM, target, sizeof(struct sockaddr))) == -1)
  {
    perror("ERROR: sendto");
    exit(1);
  }
  else if (r_val == 0)
  {
    printf("ERROR: pairing socket is closed\n");
    exit(1);
  }

  return r_val;
}

// Debug wrapper for recvfrom
int read_udp(int socket, char *buffer, sap sender, int *sender_len)
{
  int r_val, total = 0;

  while (total != BUFFLEN)
  {
    if ((r_val = recvfrom(socket, &buffer[total], (BUFFLEN - total), MSG_WAITALL,
                          sender, sender_len)) == -1)
    {
      if (errno != 11)
      {
        printf("ERROR: message might be lost/corrupted.\n");
        return -1;
      }
      else
      {
        printf("client: reached timeout (package might have been lost).\n");
        return -1;
      }
    }
    else if (r_val == 0)
    { // if client not responding
      printf("ERROR: pairing socket is closed\n");
      return -1;
    }
    else
    {
      total += r_val;
    }
  }

  return total;
}