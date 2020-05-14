#include "IncludingList.h"
#include "game.h"
#define ATTEMPS_LIMIT 3

extern HANDLE ghGameEndedEvent;
extern HANDLE ghStopEvent;
HANDLE ghPlayerQuitEvent;

//Start two threads the first one sends keyboard input from player to host 
//and the second receive from server gameboard view and display it
int startPlayerThreads( int* PORT,char * ADRESS, _Bool isHost);
DWORD WINAPI sendClientInput(void * socket);
_Bool attempToConnect(SOCKET* clientSocket, struct sockaddr_in* sa, char* ADRESS, int* PORT);
