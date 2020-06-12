#include "server.h"

int main(int argc, char *argv[])
{
  int sockfd, new_fd, pid; // listen on sock_fd and new connection on new_fd
  struct sockaddr_in server, client;
  int sin_size;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("gateway socket");
    exit(1);
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

  printf("server: waiting for client connections...\n");
  while (1)
  { // main accept() loop
    sin_size = sizeof client;
    new_fd = accept(sockfd, (struct sockaddr *)&client, &sin_size);
    if (new_fd == -1)
    {
      perror("accept");
      exit(1);
    }

    printf("server: received connection\n");
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

/*#############################################################################*/

void request_options(int socket)
{
  char buffer[BUFFLEN];

  // notify that connection is set
  strcpy(buffer, "connection is set...\n");
  write_d(socket, buffer, strlen(buffer));

  // notify that connection is set
  strcpy(buffer, "Type help for instructions\n");
  write_d(socket, buffer, strlen(buffer));

  while (1)
  {
    // Await new message from client
    printf("server awaiting new message...\n");
    read_d(socket, buffer);

    // Test which request the client asked for
    switch (strtok(buffer, " ")[0])
    {
    case '1':
      printf("adding a new movie...\n");
      add_movie(socket, buffer, strtok(NULL, "\n"));
      printf("new movie added\n");
      break;
    case '2':
      printf("removing the movie...\n");
      remove_movie(socket, buffer, strtok(NULL, "\n"));
      printf("movie removed\n");
      break;
    case '3':
      printf("retrieving titles and movie rooms...\n");
      get_movie_titles_and_rooms(socket, buffer);
      printf("all movie titles and rooms retrieved!\n");
      break;
    case '4':
      printf("retrieving movies by genre...\n");
      movies_by_genre(socket, buffer, strtok(NULL, "\n"));
      printf("movies by genre retrieved\n");
      break;
    case '5':
      printf("retrieving movie title...\n");
      get_movie_title(socket, buffer, strtok(NULL, "\n"));
      printf("movie title retrieved\n");
      break;
    case '6': // Get full movie info
      printf("retrieving movie...\n");
      get_movie(socket, buffer, strtok(NULL, "\n"));
      printf("movie sent!\n");
      break;
    case '7':
      printf("retrieving all movies...\n");
      get_all_movies(socket, buffer);
      printf("all movies retrieved!\n");
      break;
    case 'h':
      printf("sending help info...\n");
      send_help(socket, buffer);
      break;
    case 'e':
      return;
    default:
      printf("invalid option\n");
    }

    // End connection if requested by client
    if (!strcmp(buffer, "exit"))
      break;
  }

  return;
}

/*## Movie Functions ########################################################*/

void add_movie(int socket, char *buffer, char *movie_info)
{
  /* File pointer to hold reference to our file */
  FILE *movie, *index;
  int count = 0;
  char movie_info_copy[BUFFLEN], movie_name[BUFFLEN], movie_synopsis[BUFFLEN], movie_genre[BUFFLEN], movie_rooms[BUFFLEN];
  strcpy(movie_info_copy, movie_info);

  int init_size = strlen(movie_info_copy);
  char delim[] = ";";

  char *ptr = strtok(movie_info_copy, delim);

  while (ptr != NULL)
  {
    if (count == 0)
    {
      strcpy(movie_name, ptr);
    }
    else if (count == 1)
    {
      strcpy(movie_synopsis, ptr);
    }
    else if (count == 2)
    {
      strcpy(movie_genre, ptr);
    }
    else if (count == 3)
    {
      strcpy(movie_rooms, ptr);
    }

    ptr = strtok(NULL, delim);

    count++;
  }

  // First we need to update the index.txt file

  //open file in append mode
  index = fopen("data/index.txt", "a");

  if (index == NULL)
  {
    /* File not created hence exit */
    printf("Unable to open the file.\n");
    exit(1);
  }
  else
  {
    fputs(movie_name, index);
    fputc('\n', index);
  }

  fclose(index);

  // Now we need to create the new file
  char fileName[1000] = "data/";
  char fileType[5] = ".txt";

  strcat(fileName, movie_name); // concatenate folder name with typed movie name
  strcat(fileName, fileType);   // concatenate folder + movie_name with file type to generate the complete filename

  movie = fopen(fileName, "w");

  /* fopen() return NULL if last operation was unsuccessful */
  if (movie == NULL)
  {
    /* File not created hence exit */
    printf("Unable to create file.\n");
    exit(1);
  }
  else
  {
    char c = '\n';

    /* Write data to file */
    fputs(movie_name, movie);
    fputc(c, movie);

    fputs(movie_synopsis, movie);
    fputc(c, movie);

    fputs(movie_genre, movie);
    fputc(c, movie);

    fputs(movie_rooms, movie);
    fputc(c, movie);
  }

  /* Close file to save file data */
  fclose(movie);

  // Send positive response with the movie identifier to client
  char identifier[BUFFLEN] = "";
  strcat(identifier, movie_name);
  strcpy(buffer, identifier);
  write_d(socket, buffer, strlen(buffer));

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  return;
}

void remove_movie(int socket, char *buffer, char *movie_name_copy)
{
  // First we need to delete the line that contains the removed movie from index.txt
  FILE *index, *replacer;
  char filename[20] = "data/index.txt";
  char ch, movie_name[BUFFLEN], movie_to_compare[BUFFLEN];
  int delete_line = 0, temp = 1;

  strcpy(movie_to_compare, movie_name_copy);

  //open file in read mode
  index = fopen(get_path(buffer, "index", 't'), "r");

  while (fgets(movie_name, BUFFLEN, index))
  {
    movie_name[strlen(movie_name) - 1] = '\0';

    if (strcmp(movie_to_compare, movie_name) == 0)
    {
      delete_line = temp;
      break;
    }

    temp++;
  }

  ch = getc(index);
  while (ch != EOF)
  {
    ch = getc(index);
  }
  //rewind
  rewind(index);

  //open new file in write mode
  replacer = fopen("data/replica.txt", "w");
  temp = 1;
  int c = fgetc(index);

  do
  {
    if (temp != delete_line)
    {
      //copy all lines to file replica.txt
      fputc((char)c, replacer);
    }

    if ((char)c == '\n')
    {
      temp++;
    }

    // Checking for end of file
    if (feof(index))
      break;

  } while ((c = fgetc(index)) != EOF);

  fclose(index);
  fclose(replacer);
  remove(filename);

  //rename the file replica.txt to original name
  rename("data/replica.txt", filename);

  // Now we need to delete the txt file for the chosen movie
  char fileName[1000] = "data/";
  char fileType[5] = ".txt";

  strcat(fileName, movie_to_compare); // concatenate folder name with typed movie name
  strcat(fileName, fileType);         // concatenate folder + movie_name with file type to generate the complete filename

  if (remove(fileName) == 0)
    printf("Deleted successfully\n");
  else
  {
    printf("Unable to delete the file\n");
    exit(1);
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof
  return;
}

void get_movie_titles_and_rooms(int socket, char *buffer)
{
  FILE *index, *movie;
  char movie_name[BUFFLEN];

  index = fopen(get_path(buffer, "index", 't'), "r");

  while (fgets(movie_name, BUFFLEN, index))
  {
    movie_name[strlen(movie_name) - 1] = '\0';
    movie = fopen(get_path(buffer, movie_name, 't'), "r");

    get_line(movie, buffer, 1);
    strcat(buffer, "\nSalas de exibição: ");
    get_line(movie, &buffer[strlen(buffer)], 4);
    strcat(buffer, "\n");
    write_d(socket, buffer, strlen(buffer));

    fclose(movie);
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  fclose(index);
  return;
}

void movies_by_genre(int socket, char *buffer, char *genre_copy)
{
  FILE *index, *movie;
  char genre[BUFFLEN], movie_name[BUFFLEN];
  int helper = 0; // flag to check if a movie from that genre exists

  strcpy(genre, genre_copy);
  index = fopen(get_path(buffer, "index", 't'), "r");

  while (fgets(movie_name, BUFFLEN, index))
  {
    movie_name[strlen(movie_name) - 1] = '\0';
    movie = fopen(get_path(buffer, movie_name, 't'), "r");
    get_line(movie, buffer, 3);
    printf("%s is of the genre |%s|%s|\n", movie_name, buffer, genre);

    if (!strcmp(buffer, genre))
    {
      get_line(movie, buffer, 1);
      strcat(buffer, "\nGênero: ");
      get_line(movie, &buffer[strlen(buffer)], 3);
      strcat(buffer, "\n");
      write_d(socket, buffer, strlen(buffer));
      helper = 1;
    }

    fclose(movie);
  }

  if (helper == 0)
  {
    char no_movie[60] = "There is no movies from that genre inside the system :(\n";
    write_d(socket, no_movie, strlen(no_movie)); // send a message that no movie with that genre exists
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  fclose(index);
  return;
}

void get_movie_title(int socket, char *buffer, char *movie_name)
{
  FILE *movie;
  char path[BUFFLEN];
  char helper[14] = " <- Título\n";

  movie = fopen(get_path(path, movie_name, 't'), "r");

  get_line(movie, buffer, 1); // Get movie title
  write_d(socket, strcat(buffer, helper), strlen(buffer) + strlen(helper));

  write_d(socket, buffer, 0); // Send empty buffer to sinal eof

  fclose(movie);
  return;
}

void get_all_movies(int socket, char *buffer)
{
  FILE *index;

  get_path(buffer, "index", 't');
  index = fopen(buffer, "r");

  while (fgets(buffer, BUFFLEN, index))
  {
    buffer[strlen(buffer) - 1] = '\0';
    printf("sending movie: %s\n", buffer);
    write_d(socket, buffer, strlen(buffer)); // send movie name
    get_movie(socket, buffer, buffer);       // send movie
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  return;
}

void get_movie(int socket, char *buffer, char *buff_movie_name)
{
  FILE *fptr;
  int line = 0;
  char movie_name[BUFFLEN], tag[BUFFLEN];
  char *tags[] = {"Nome: \0", "Sinopse: \0", "Gênero: \0", "Salas de exibição: \0", "                                 \0"};

  strcpy(movie_name, buff_movie_name); // Copy movie name key from buffer

  // Gets the values in the txt file
  get_path(buffer, movie_name, 't');

  if ((fptr = fopen(buffer, "r")) == NULL)
  {
    printf("Error! opening file: %s\n", buffer);
    exit(1); // Exits if failed to open file
  }

  // Send contents from file
  while (fgets(buffer, BUFFLEN, fptr))
  {
    strcpy(tag, tags[line]);
    strcat(tag, buffer);
    write_d(socket, tag, strlen(tag));
    if (line < 6)
      ++line;
  }

  write_d(socket, buffer, 0); // Send empty buffer to sinal eof

  return;
}

void send_help(int socket, char *buffer)
{
  FILE *help;

  get_path(buffer, "help", 't');
  help = fopen(buffer, "r");

  while (fgets(buffer, BUFFLEN, help))
  {
    buffer[strlen(buffer) - 1] = '\0';
    printf("sending: %s\n", buffer);
    write_d(socket, buffer, strlen(buffer));
  }

  write_d(socket, buffer, 0); // Send empty buffer to signal eof

  return;
}

/*## Transfer file functions ##################################################*/

// Function to split all the files that are inside "data" folder and send
// them to the client
void send_file(int socket, char *buffer, char *full_path)
{
  FILE *input;          // File that is going to be sent
  long int i = 0, size; // size of it

  input = fopen(full_path, "rb");
  printf("sending file \"%s\"\n", get_name(full_path));

  // Get file size
  fseek(input, 0, SEEK_END);
  size = ftell(input);
  fseek(input, 0, SEEK_SET);

  sprintf(buffer, "%ld", size);            // Convert size to string
  write_d(socket, buffer, strlen(buffer)); // Send to client

  while (i < size)
  { // reads char by char filling buffer until eof
    buffer[(i++) % BUFFLEN] = fgetc(input);
    if (i % BUFFLEN == 0 || i == size)
      write_d(socket, buffer, BUFFLEN); // sends entire buffer to avoid border issues
  }

  printf("file sent\n");
  fclose(input);
  return;
}

//## Functions to handle paths ##################################################

// Function to get the path of the file that will be sent
char *get_path(char *path, char *file_name_buff, char id)
{
  char szTmp[32], file_name[BUFFLEN];
  int bytes;

  strcpy(file_name, file_name_buff);
  sprintf(szTmp, "/proc/%d/exe", getpid()); // get this process origin file path
  bytes = readlink(szTmp, path, BUFFLEN);   // save path

  for (bytes; path[bytes] != '/'; --bytes)
    ;                     // removes the process name
  path[bytes + 1] = '\0'; // end of file

  if (id == 't')
    strcat(strcat(strcat(path, "data/"), file_name), ".txt");

  return path; // return the path and its size
}

char *get_name(char *path)
{
  int i;

  for (i = strlen(path); i >= 0; --i)
    if (path[i] == '/')
      return &(path[i + 1]);

  return path;
}

/*## Functions to search something specific inside a file #######################*/

// Retrieves a specific entry from the movie
char *get_line(FILE *movie, char *buffer, int line)
{
  int i, position = ftell(movie);

  fseek(movie, 0, SEEK_SET);
  for (i = 1; i < line; ++i)
    fgets(buffer, BUFFLEN, movie);
  buffer = fgets(buffer, BUFFLEN, movie);
  fseek(movie, position, SEEK_SET);

  if (buffer && buffer[strlen(buffer) - 1] == '\n')
    buffer[strlen(buffer) - 1] = '\0';

  return buffer;
}
