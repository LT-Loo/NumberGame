// Header file that lists all socket functions (both server and client)

#ifndef SOCKET_H
#define SOCKET_H

int createServerSoc(int portNum);
int acceptConnection(int serverSoc, int n, int playerNum);
int createPlayerSoc(int portNum, char *hp);

#endif