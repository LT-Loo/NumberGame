// Function codes of all socket functions (both server and client)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 1024
#define TIMEOUT 30

// Create socket for game server
int createServerSoc(int portNum) {
    // Create socket
    int serverSoc = socket(AF_INET, SOCK_STREAM, 0); 
    if (serverSoc < 0) {
        printf("ERROR Fail to create game server socket.\n");
        exit(0);
    } else {printf("Game server socket successfully created.\n");}

    // Get hostname
    struct hostent *host;
    char hostname[BUF_SIZE];
    gethostname(hostname, BUF_SIZE);
    host = gethostbyname(hostname);

    // Build IP address
    struct sockaddr_in gameAddr;
    bzero(&gameAddr, sizeof(gameAddr));
    bcopy(host -> h_addr, &gameAddr.sin_addr.s_addr, host -> h_length);
    gameAddr.sin_port = htons(portNum);
    gameAddr.sin_family = AF_INET;

    // Bind IP address to socket
    if (bind(serverSoc, (struct sockaddr*)&gameAddr, sizeof(gameAddr)) < 0) {
        printf("ERROR Fail to bind IP address.\n");
        exit(0);
    } else {printf("Bind IP address success.\n");}

    if (listen(serverSoc, 0) < 0) { // Wait for incoming connection request
        printf("ERROR Fail to wait for connection.\n");
        exit(0);
    } else {printf("Waiting for player...\n");}

    return serverSoc;
}


// Accept connection request and set timeout for player
int acceptConnection(int serverSoc, int n, int playerNum) {
    // Accept connection
    struct sockaddr_in playerAddr;
    int playerLen = sizeof(playerAddr);
    int playerSoc = accept(serverSoc, (struct sockaddr*)&playerAddr, &playerLen);
    if (playerSoc < 0) {
        printf("ERROR Fail to accept player connection.\n");
        return -1;
    }
    else { // If connection established, show information message on server screen
        printf("Connection with player %d established.\nNumber of player: %d/%d... ", n, n, playerNum);
        if (n < playerNum) {printf("Waiting for player...\n");}
        else if (n == playerNum) {printf("Start game!\n\n");}
    }

    // Set timeout
    struct timeval timeOut;
    timeOut.tv_sec = TIMEOUT;
    timeOut.tv_usec = 0;
    if (setsockopt(playerSoc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeOut, sizeof(timeOut)) < 0) {
        printf("ERROR Fail to track time.\n");
        return -1;
    }

    return playerSoc;
}


// Create socket for client server (player)
int createPlayerSoc(int portNum, char *hp) {
    int playerSoc = socket(AF_INET, SOCK_STREAM, 0); // Create client socket
    if (playerSoc < 0) {
        printf("ERROR: Fail to create player socket.\n");
        return -1;
    } else {printf("Player socket successfully created.\n");}

    // Get IP address from system name
    struct hostent *host = gethostbyname(hp);
    struct sockaddr_in playerAddr;
    bzero(&playerAddr, sizeof(playerAddr));
    if (host == NULL) {
        printf("ERROR Fail to get host IP.\n");
        return -1;
    } else {printf("Host IP successfully found.\n");}
    bcopy(host -> h_addr, (struct sockaddr*)&playerAddr.sin_addr.s_addr, host -> h_length);
    playerAddr.sin_family = AF_INET;
    playerAddr.sin_port = htons(portNum);

    // Connect player to server
    if (connect(playerSoc, (const struct sockaddr*)&playerAddr, sizeof(playerAddr)) < 0) {
        printf("ERROR: Fail to connect game server.\n");
        return -1;
    } else {printf("Connection with game server established.\n");}

    return playerSoc;
}
