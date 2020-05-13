#include "IncludingList.h"

extern HANDLE ghStopEvent;

DWORD WINAPI serverThread(int * PORT);
void serverLoop(SOCKET* connectionSocket);
int startServer(HANDLE* serverHandle, int * PORT);
void receiveDataFromPlayers(SOCKET* connectionSocket);

struct PlayerData {
	SOCKET* socket;
	int ID;
	char * nickName;
	char lastMove;
}typedef PlayerData;