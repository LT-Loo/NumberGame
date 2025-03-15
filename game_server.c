// Main Server Program

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>

#include "socket.h"
#include "game_functions.h"

#define BUF_SIZE 1024

int main(int argc, char* argv[]) {
    char gameBuf[BUF_SIZE];
    char playerBuf[BUF_SIZE];
    char *text = "TEXT ", *go = "GO", *end = "END", *error = "ERROR"; // Protocol

    if (argc != 4) { // Check validity of arguments
        printf("ERROR Invalid number of arguments.\n");
        exit(0);
    }

    int playerNum = atoi(argv[3]); // Get number of players from argument
    if (playerNum <= 1) { // Check if number of players valid
        printf("ERROR Invalid number of players.\n");
        exit(0);
    }
    int currentNum = playerNum; // Number of players during game execution
    int players[playerNum]; // Array to store player(client) sockets

    int gameSoc = createServerSoc(atoi(argv[1])); // Create game server socket
    if (gameSoc < 0) {exit(0);}

    // Wait and accept connections from players
    for (int i = 0; i < playerNum; i++) {
        players[i] = acceptConnection(gameSoc, i+1, playerNum);
        if (players[i] > 0) {welcomeMsg(players, i, playerNum, gameSoc);} // Send welcome message if connection established
        else { 
            char errorMsg[BUF_SIZE];
            sprintf(errorMsg, "ERROR Server fails to accept player %d.\n", i + 1);
            closeServer(players, gameSoc, 1, i, errorMsg); // Terminate server when fails to accept connection
        }
    }

    // Game execution
    int i = 0; // Player's turn
    int sum = 0, chance = -1, winner;
    while (sum < 30) {

        char message[BUF_SIZE];
        memset(playerBuf, '\0', sizeof(playerBuf));
        memset(gameBuf, '\0', sizeof(gameBuf));

        for (int j = 0; j < playerNum; j++) { // Request input from player and send waiting message to others
            if (players[j] > 0) {requestInput(players, j, i, playerNum, sum, chance, gameSoc);}
        }
        
        // Receive message from player
        if (recv(players[i], playerBuf, BUF_SIZE, 0) < 0) {
            if (errno == EAGAIN) { // If player does not response in 30 seconds, disconnect player
                char *quitMsg = "\nFail to enter number within 30 seconds. You are forced out the game.\n";
                char informMsg[BUF_SIZE];
                sprintf(message, "Player %d fails to enter number within time limit. Player %d quits game.\n%d player(s) left. ", i + 1, i + 1, --currentNum);
                quitPlayer(players, playerNum, i, currentNum, quitMsg, informMsg, gameSoc);

            }
            else {printf("ERROR: Fail to receive message from player %d.\n", i + 1);}
        }
        else { // Message successfully accepted from player
            if (strncmp(playerBuf, "MOVE ", strlen("MOVE ")) == 0) { // "MOVE" message
                strcpy(playerBuf, &playerBuf[strlen("MOVE ")]);
                int num = atoi(playerBuf);

                // If player sends wrong input (Game error)
                if (chance != 1 && (num < 1 || num > 9)) { // Reduce chances
                    if (chance < 0) {chance = 4;}
                    else {chance--;}
                    continue;
                }
                else if (chance == 1 && (num < 1 || num > 9)) { // If last chance, disconnect player
                    char *quitMsg = "ERROR: Invalid number. No chance left. You are forced out of the game.\n";
                    char informMsg[BUF_SIZE];
                    sprintf(informMsg, "Player %d fails to enter valid number within 5 chances. Player %d quits game.\n%d player(s) left. ", i + 1, i + 1, --currentNum);
                    quitPlayer(players, playerNum, i, currentNum, quitMsg, informMsg, gameSoc);
                    chance = -1;
                }
                else {sum += num; chance = -1;} // Valid number from player, add score
            }

            else if (strcmp(playerBuf, "QUIT") == 0) { // "QUIT" message, disconnect player
                char *quitMsg = "Quit game.\n";
                char informMsg[BUF_SIZE];
                sprintf(informMsg, "Player %d quits game. %d player(s) left. ", i + 1, --currentNum);
                quitPlayer(players, playerNum, i, currentNum, quitMsg, informMsg, gameSoc);
            }

            else { // If protocol error, disconnect player
                printf("%s Invalid message received from player %d (Fail to follow protocol).\n", error, i + 1);
                char quitMsg[BUF_SIZE];
                char informMsg[BUF_SIZE];
                sprintf(quitMsg, "%s Invalid message sent (Fail to follow protocol).\n", error);
                sprintf(informMsg, "Player %d disconnected. %d player(s) left.\n", i + 1, --currentNum);
                printf("%s", quitMsg);
                quitPlayer(players, playerNum, i, currentNum, quitMsg, informMsg, gameSoc);
            }
        }

        if (sum >= 30) {winner = i;} // If more than 30 scores obtained, end game

        if (currentNum == 1) {winner = i; break;} // If only one player left

        // Iterates among player
        if (chance < 0) {i++;} 
        if (i >= playerNum) {i = 0;}
        while (players[i] < 0) {
            i++;
            if (i >= playerNum) {i = 0;}
        }
    }

    // Announce winner and results
    printf("\n\nGame end!\nWinner: Player %d     Total Scores: %d\n", winner + 1, sum); // Show results on server side
    for (int j = 0; j < playerNum; j++) {
        if (players[j] > 0) {
            strcpy(gameBuf, text);
            char message[BUF_SIZE];

            // Send different messages to winner and other players
            if (j == winner) {sprintf(message, "\nCongratulations! You Won!\nThe final sum is %d.\nThank you for playing the game! Bye!\n", sum);}
            else {sprintf(message, "\nSorry, you lost!\nPlayer %d won the game. The final sum is %d.\nThank you for playing the game! Bye!\n", winner, sum);}
            if (send(players[j], strcat(gameBuf, message), sizeof(gameBuf), 0) < 0) {
                sprintf(message, "ERROR Server fails to send TEXT message to player %d.\n", j + 1);
                closeServer(players, gameSoc, 1, 1, message);
            }

            if (send(players[j], end, sizeof(end), 0) < 0) { // Send "END" message
                sprintf(message, "ERROR Server fails to send TEXT message to player %d.\n", j + 1);
                closeServer(players, gameSoc, 1, 1, message);
            }
        }
    }

    // Close server program
    printf("\nClosing game...\n");
    if (close(gameSoc) < 0) {
        printf("ERROR: Fail to close game server.\n");
        exit(0);
    } else {printf("Game server closed successfully.\n");}

    return 0;
}