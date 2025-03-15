## Introduction
This is game where players take turns to enter a number between 1 and 9, and the input number will be added into the score, which is the sum of all inputs numbers previously selected by all players. The first player to get a total of 30 scores or more wins the game.

## Requirements
Have Linux/Cygwin environment installed in your machine.

[Optional]
Run two independent programs on different machine at the same time and communicate through a network. However, they can also be run on the same machine in separate terminals.

## Run Program
1. Launch two or more terminals with Linux/Cygwin environment (can be on different machines or same machine).
2. Run `game_server.o` on one of the terminals with appropriate arguments as required [here](#game-server-program).
3. Run `game_client.o` on the rest terminals with appropriate arguments as required [here](#player-client-program).
4. Interact with the server program by entering valid input on client programs.

## Game Stage
This game is divided into 4 stages:

1. Game Server Initiation - Launch the game server with appropriate arguments.

2. Join (Connection Establihsed) - The game server waits for players to join. Once required number of players has joined, the server then proceeds to the next stage.

3. Play (Game Execution) - The game server iterates amongst the players in their respective joining order. Game server and players interact with each other by sending or receiving messages. Note that no new players are allowed to join at this phase. However, players can choose to leave the game. The game server will end the game if the number of players staying in the game falls below required minimum.

4. Game Termination - The game server sends a message to each player announcing the winner and final score. Afterward, the connections between the game server and client servers are terminated, and all sockets are closed.

## Game Server Program
The command to run the server program is given below:
```
game_server <Port Number> <Game Type> <Min Player Number>
``` 
where `Game Type` should be "numbers".<br><br>

There are 4 types of messages that can be sent by the server to the client programs:
1. TEXT <text> - Client needs to print the contents of <text> ("TEXT" omitted) to the screen once received the message.

2. GO - This message tells the client to wait for input from the player while the server waits to receive the input from the client.

3. END - This message indicates the end of a game. Client that receives this message should disconnect from the server and stop running.

4. ERROR - This message indicates an error occurs.

## Player Client Program
The command to run the server program is written below:
```
game_client <Game Type> <Server Name> <Port Number>
```
where `Game Type` should be "numbers" and `Server Name` is the machine/computer name which the server program is running on.<br><br>

There are 4 types of messages that can be sent by the client program to the server:
1. MOVE <move> - This is a response made by client to a "GO" message. <move> indicates the number to be added into the total score, must be in the range of 1 to 9.

2. QUIT - Tells the server program that the player wants to quit the game. This message can only be sent as a response to a "GO" message.

## Developer
Loo<br>
loo.workspace@gmail.com
