#makefile for client and server

all: myclient myserver

myclient: client.c
	gcc -o myclient client.c

myserver: server.c
	gcc -o myserver server.c