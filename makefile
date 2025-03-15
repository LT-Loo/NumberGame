# Makefile for both client and server programs

all: server client

server: game_server.o socket.o game_functions.o
	cc game_server.o socket.o game_functions.o -o game_server

client: game_client.o socket.o
	cc game_client.o socket.o -o game_client

game_server.o: game_server.c socket.h game_functions.h
	cc -c game_server.c

game_client.o: game_client.c socket.h
	cc -c game_client.c

socket.o: socket.c
	cc -c socket.c
