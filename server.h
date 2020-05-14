#include "IncludingList.h"
#include "game.h"
extern HANDLE ghStopEvent;

struct PlayerData {
	SOCKET* socket;
	int ID;
	char* nickName;
	char lastMove;
}typedef PlayerData;

DWORD WINAPI serverThread(int * PORT);
int startServer(HANDLE* serverHandle, int * PORT);
DWORD WINAPI receiveDataFromPlayer(void * data);
DWORD WINAPI sendingDataToPlayer(void* data);
