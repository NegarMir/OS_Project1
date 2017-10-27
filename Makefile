CC = gcc
#This target will compile all files
all : client.c server.c
	gcc server.c client.c && echo success

server.out : server.cpp
	gcc server.c && echo succes

client.out: client.c
	gcc client.c && echo success
