// Function codes of all functions related to the game (when executing)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#define BUF_SIZE 1024

// Is invoked when server error occurs
void closeServer(int players[], int gameSoc, int error, int empty, char* message) {
    int n;
    char errorMsg[BUF_SIZE];

    // Send "ERROR" and "END" message to inform all players to disconnect
    strcpy(errorMsg, "TEXT ");
    if (empty > 0) { // If there are already connected players (Possible of no connected player during joining phase)
        n = sizeof(*players) / sizeof(players[0]);
        for (int i = 0; i < n; i++) {
            if (players[i] > 0) {
                if (error == 1) {
                    if (send(players[i], strcat(errorMsg, message), sizeof(errorMsg), 0) < 0) {
                        printf("ERROR Fail to send END message to all players.\n");
                        exit(1);
                    }
                }
                if (send(players[i], "END", sizeof("END"), 0) < 0) {
                    printf("ERROR Fail to send END message to all players.\n");
                    exit(1);
                }
                close(players[i]); // Close players' connections
            }
        }
    }

    // Terminate server program
    if (error == 1) {printf("%s", message);}
    printf("\nTerminating program...\n");
    if (close(gameSoc) < 0) {
        printf("ERROR Fail to close game server.\n");
        exit(1);
    } 
    else {printf("Game server terminated.\n");}
    exit(0);
}


// Send welcome and informative message to player every time a connection is established
void welcomeMsg(int players[], int current, int playerNum, int gameSoc) {
    for (int j = 0; j <= current; j++) {
        char wait[BUF_SIZE];
        char message[BUF_SIZE];

        strcpy(message, "TEXT ");
        if (j == current) { // If newly joined player, send welcome message
            strcat(message, "Welcome to the game. ");
            if (current + 1 < playerNum) {sprintf(wait, "You are Player %d.\nWaiting for %d more player(s)...\n", current + 1, playerNum - (current + 1));}
            else {sprintf(wait, "You are Player %d.\nRoom full. Start game!\n", current + 1);}
        }
        else { // If already joined player, send informative message
            if (current + 1 < playerNum) {sprintf(wait, "Player %d enters game. Waiting for %d more player(s)...\n", current + 1, playerNum - (current + 1));}
            else {sprintf(wait, "Player %d enters game. Room full. Start game!\n", current + 1);}
        }
        if (send(players[j], strcat(message, wait), sizeof(message), 0) < 0) {
            sprintf(message, "ERROR from server: Fail to send welcome message to player %d.\n", j + 1);
            closeServer(players, gameSoc, 1, 1, message); // If server error occurs
        }
    }
}


// Send "GO" message to request player enter number
void requestInput(int players[], int i, int current, int playerNum, int sum, int chance, int gameSoc) {
    char gameBuf[BUF_SIZE];
    char message[BUF_SIZE];
    char *text = "TEXT ", *go = "GO";
    strcpy(gameBuf, text);

    if (i == current) { // If current player, send "TEXT" message according to number of chances left
        if (chance < 0) {sprintf(message, "Your turn now. You have 30 seconds.\nSum is %d. Please Enter number(1-9): ", sum);}
        else if (chance > 0) {sprintf(message, "GAME ERROR Invalid number. You have %d chance(s) left.\nTry again. Enter number(1-9): ", chance);}
        if (send(players[current], strcat(gameBuf, message), sizeof(gameBuf), 0) < 0) {
            sprintf(message, "ERROR Server fails to send TEXT message to player %d.\n", current + 1);
            closeServer(players, gameSoc, 1, 1, message); // If server error occurs
        } 

        if (send(players[current], go, sizeof(go), 0) < 0) { // Send "GO" message
            sprintf(message, "ERROR Server fails to send GO message to player %d.\n", current + 1);
            closeServer(players, gameSoc, 1, 1, message); // If server error occurs
        }
    }
    else { // If waiting player, send "TEXT" message according to number of chances left
        if (chance < 0) {
            sprintf(message, "Player %d's turn now. Please wait for your turn.\n", current + 1);
            printf("Player %d's turn now.\n", current + 1); // Informative message is also shown on server side
        }
        else if (chance > 0) {
            sprintf(message, "Invalid number entered, Player %d's turn remain. Please wait for your turn.\n", current + 1);
            printf("Invalid number entered, Player %d's turn remain.\n", current + 1); // Game error message is also shown on server side
        }
        if (send(players[i], strcat(gameBuf, message), sizeof(gameBuf), 0) < 0) {
            sprintf(message, "ERROR Server fails to send TEXT message to waiting players.\n");
            closeServer(players, gameSoc, 1, 1, message); // If server error occurs
        }
    }           
}


// Is invoked to disconnect current player
void quitPlayer(int players[], int playerNum, int quitPlayer, int currentNum, char *quitMsg, char *informMsg, int gameSoc) {
    char gameBuf[BUF_SIZE];
    char message[BUF_SIZE];
    char *text = "TEXT ", *end = "END", *error = "ERROR";

    printf("%s", informMsg); // Show information on server side

    strcpy(gameBuf, text);
    for (int k = 0; k < playerNum; k++) {
        strcpy(gameBuf, text);
        if (k == quitPlayer && players[quitPlayer] > 0) { // If player to disconnect
            if (send(players[quitPlayer], strcat(gameBuf, quitMsg), sizeof(gameBuf), 0) < 0) { // Send "TEXT" error message
                sprintf(message, "ERROR Server fails to send TEXT message to player %d.\n", quitPlayer + 1);
                closeServer(players, gameSoc, 1, 1, message); // If server error occurs
            } 
            if (send(players[quitPlayer], end, sizeof(end), 0) < 0) { // Send "END" message
                sprintf(message, "ERROR Server fails to send END message to  player %d.\n", quitPlayer + 1);
                closeServer(players, gameSoc, 1, 1, message); // If server error occurs
            }
            players[quitPlayer] = -1; // Indicate that this player has disconnected from the server
        }

        else if (players[k] > 0) { // If waiting player, send informative "TEXT" message
            strcpy(message, informMsg);
            if (currentNum == 1) {strcat(message, "Game end.\n");}
            else {strcat(message, "Continue game.\n");}
            if(send(players[k], strcat(gameBuf, message), sizeof(gameBuf), 0) < 0) {
                sprintf(message, "ERROR Server fails to send TEXT message to all players.\n");
                closeServer(players, gameSoc, 1, 1, message); // If server error occurs
            }
        }                
    }
}