// Main Client Program (Player)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include "socket.h"

#define BUF_SIZE 1024
#define TIMEOUT 30 * 1000

int main(int argc, char *argv[]) {
    char playerBuf[BUF_SIZE];
    char serverBuf[BUF_SIZE];
    char *move = "MOVE ", *quit = "QUIT"; // Protocol

    if (argc != 4) { // Check validity of number of arguments
        printf("ERROR: Invalid number of arguments.\n");
        exit(0);
    }
    
    int player = createPlayerSoc(atoi(argv[3]), argv[2]); // Create player socket
    if (player < 0) { exit(0);}

    // For checking response time purpose
    struct pollfd input;
    input.fd = 0;
    input.events = POLLIN;

    // Game execution
    while(1) {
        memset(playerBuf, '\0', sizeof(playerBuf));
        memset(serverBuf, '\0', sizeof(serverBuf));

        if (recv(player, serverBuf, BUF_SIZE, 0) < 0) { // Receive message from game server
            printf("ERROR: Fail to receive message.\n");
            break;
        }
        else {
            if (strncmp(serverBuf, "TEXT", strlen("TEXT")) == 0) { // If "TEXT" message
                strcpy(serverBuf, &serverBuf[strlen("TEXT ")]);
                printf("\n%s", serverBuf);
                fflush(stdout);
                if (strncpy(serverBuf, "ERROR", sizeof("ERROR")) == 0) {break;} // Stop game and disconnect once receive server error message
            }

            else if (strcmp(serverBuf, "GO") == 0) { // If "GO" message
                char command[BUF_SIZE];
                if (poll(&input, 1, TIMEOUT) < 0) {
                    printf("ERROR: Fail to call poll().\n");
                    break;
                } 
                if ((input.revents && POLLIN) != 0) { // Unblock fgets() and receive input if available
                    fgets(command, BUF_SIZE, stdin);
                    command[strlen(command) - 1] = '\0';
                    for (int i = 0; i < strlen(command); i++) {command[i] = tolower(command[i]);}

                    if (strcmp(command, "quit") == 0) {strcpy(playerBuf, quit);} // Create "QUIT" message
                    else { // Create "MOVE" message
                        strcpy(playerBuf, move);
                        strcat(playerBuf, command);
                    }

                    if (send(player, playerBuf, sizeof(playerBuf), 0) < 0) { // Send message
                        printf("ERROR: Fail to send message.\n");
                        exit(0);
                    }    
                } else {continue;}  
            }

            else if (strcmp(serverBuf, "END") == 0) {break;} // If "END" message, disconnect and close program

            else if (strncmp(serverBuf, "ERROR", strlen("ERROR")) == 0) { // If "ERROR" message
                printf("\n%s", serverBuf);
                fflush(stdout);
                break;
            }

            else { // If invalid server message received
                printf("\nERROR Invalid message received from game server.\n");
                break;
            }
        }
    }

    // Disconnecting from game server and terminate program
    printf("\nDisconnecting from game server...\n");
    if (close(player) < 0) {
        printf("ERROR: Fail to disconnect from game server.\n");
        exit(0);
    } else {printf("Disconnected from game server. Closing program...\n");}
    sleep(2);
    printf("Program terminated.\n");
    
    return 0;
}