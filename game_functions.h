// Header file that lists all functions related to the game (when executing)

#ifndef GAME_FUNCTIONS_H
#define GAME_FUNCTIONS_H

void closeServer(int players[], int gameSoc, int error, int empty, char* message);
void welcomeMsg(int players[], int current, int playerNum, int gameSoc);
void requestInput(int players[], int i, int current, int playerNum, int sum, int chance, int gameSoc);
void quitPlayer(int players[], int playerNum, int quitPlayer, int currentNum, char *quitMsg, char *informMsg, int gameSoc);

#endif