# variables
CC=gcc -o
RM=rm -f

all: server/server client/client

server: server/server.c
	$(CC) server/server server/server.c

client: client/client.c
	$(CC) client/client client/client.c

clean:
	$(RM) server/server 
	$(RM) client/client