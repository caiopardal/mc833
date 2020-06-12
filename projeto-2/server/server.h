#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFLEN 2048  // Length of the message buffer
#define UDP_PORT 8080 // the UDP port users will be connectiog to

// FUNCTIONS signatures
void request_options(int);
void send_file(int, char *, char *);
void send_help(int, char *);
char *get_name(char *);
char *get_line(FILE *, char *, int);
char *get_path(char *, char *, char);
void get_all_movies(int, char *);
void get_movie_titles_and_rooms(int, char *);
void get_movie(int, char *, char *);
void add_movie(int, char *, char *);
void remove_movie(int, char *, char *);
void movies_by_genre(int, char *, char *);
void get_movie_title(int, char *, char *);
void send_data(int, char *, int);

// UDP WRAPPERS ///////////////////////////////////////////////
// Debuggin wrapper for sendto
int write_udp(int socket, char *buffer, int length, struct sockaddr_in target, int sender_len)
{
  int i, r_val;

  // Fill message to standard size of buffer
  for (i = length; i < BUFFLEN; ++i)
    buffer[i] = '\0';

  if ((r_val = sendto(socket, (const char *)buffer, BUFFLEN, MSG_CONFIRM, (const struct sockaddr *)&target, sender_len)) == -1)
  {
    printf("ERROR: client info might have been lost.\n");
    return -1;
  }
  else if (r_val == 0)
  {
    printf("ERROR: pairing socket is closed\n");
    return -1;
  }

  return r_val;
}

// Debuggin wrapper for receive from udp server
int read_udp(int socket, char *buffer, struct sockaddr_in *sender, int *sender_len)
{
  int r_val, total = 0;

  while (total != BUFFLEN)
  {
    if ((r_val = recvfrom(socket, &buffer[total], (BUFFLEN - total), MSG_WAITALL,
                          (struct sockaddr *)sender, sender_len)) == -1)
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