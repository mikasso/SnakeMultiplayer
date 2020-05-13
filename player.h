#include "IncludingList.h"
#include "game.h"
#define ATTEMPS_LIMIT 3

extern HANDLE ghStopEvent;

//Start two threads the first one sends keyboard input from player to host 
//and the second receive from server gameboard view and display it
int startPlayerThreads( int* PORT,char * ADRESS, _Bool isHost);
void sendClientInput(SOCKET* clientSocket);
_Bool attempToConnect(SOCKET* clientSocket, struct sockaddr_in* sa, char* ADRESS, int* PORT);
