#pragma once
#include "IncludingList.h"
#include "game.h"
#include "player.h"

extern HANDLE ghGameEndedEvent;
extern HANDLE ghStopEvent;
extern HANDLE ghPlayerQuitEvent;
extern HANDLE ghBoardSemaphore;
extern HANDLE ghLoopDone;
extern HANDLE ghGameReady;
extern _Bool gameStatus;
HANDLE* ghPlayersReceivedEvent;
HANDLE * nickNamesReceived;
#define CONNECTED 1
#define DISCONNECTED -1
#define MAX_LEN 100

// Player Handling
struct PlayerData {	//Ta struktura jest do petli w grze
	int ID;				//Id gracza 
	char * nickName;		//jego nickname opcjonalnie moge to zaimplementowac 
	char lastMove;		//ostatni ruch gracza TO POWINNA BYC JAKAS KOLEJKA WLASCIWIE
	_Bool alive;
	_Bool connected;
	int score;
	int x[MAX_LEN], y[MAX_LEN];
	int size;
	char color;
	HANDLE playerSemaphore;
}typedef PlayerData;

struct PlayerShowData {
	int ID;
	char * nickName;
	int score;
	_Bool alive;
}typedef PlayerShowData;

struct PlayerServerInfo {//Ta natomiast do obslugiwania gracza przez serwer
	PlayerData * data;
	int count;								//liczba graczy
	struct PlayerServerInfo ** othersServerInfo;	//informacje o nich
	SOCKET * socket;
	HANDLE * receivingThread;
	HANDLE * sendingThread;
	struct sockaddr_in * sockAddr;
	int status;
}typedef PlayerServerInfo;

PlayerServerInfo* initPlayerServerInfo();			//Alokuje pamiec na strukture gracze w serwerze
void freePlayerServerInfo(PlayerServerInfo* info);	//Zwalnia te pamiec
void showPlayerInfo(PlayerServerInfo* player);		//Wyswietla informacje IP i ID gracza

//Server stuff
struct ServerBasicData {
	SOCKET socket;									//Gniazdko nasluchujace przychodzace polaczenia
	struct sockaddr_in addr_in;						//Informacja o adresie serwera
	int maxPlayers;									//Maksymalna libcza graczy
}typedef ServerBasicData;

struct GameData {
	PlayerServerInfo** players;
	int count;
}typedef GameData;

HANDLE* startServer(int* PORT, int maxPlayers);		//Przygotowuje watek serwera i wywoluje go
DWORD WINAPI serverThread(void* data);				//Glowny Watek serwera


//Threads Handling
HANDLE* startReceivingThreads(PlayerServerInfo** players, int connected);	//Rozpocznya watki do odbierania danych
HANDLE* startSendingThreads(PlayerServerInfo** players, int connected);		//Rozpoczyna watki do wysylania danych
DWORD WINAPI receiveDataFromPlayer(void * data);							//Watek otrzymywania danych - data powinna wskazywac
DWORD WINAPI sendingDataToPlayer(void* data);								//na struct PlayerServerInfo *

