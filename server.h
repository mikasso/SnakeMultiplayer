#include "IncludingList.h"
#include "game.h"

extern HANDLE ghStopEvent;

// Player Handling
struct PlayerData {
	int ID;
	char* nickName;
	char lastMove;
}typedef PlayerData;

struct PlayerServerInfo {
	PlayerData * data;
	SOCKET * socket;
	HANDLE * receivingThread;
	HANDLE * sendingThread;
	struct sockaddr_in * sockAddr;
}typedef PlayerServerInfo;

PlayerServerInfo* initPlayerServerInfo();
void freePlayerServerInfo(PlayerServerInfo* info);
void showPlayerInfo(PlayerServerInfo* player);

//Server stuff
struct ServerBasicData {
	SOCKET socket;
	struct sockaddr_in addr_in;
	int maxPlayers;
}typedef ServerBasicData;

HANDLE* startServer(int* PORT, int maxPlayers);
DWORD WINAPI serverThread(void* data);


//Threads Handling
HANDLE* startReceivingThreads(PlayerServerInfo** players, int connected);
HANDLE* startSendingThreads(PlayerServerInfo** players, int connected);
DWORD WINAPI receiveDataFromPlayer(void * data);
DWORD WINAPI sendingDataToPlayer(void* data);

