#include "IncludingList.h"
#include "game.h"
#include "player.h"

extern HANDLE ghGameEndedEvent;
extern HANDLE ghStopEvent;
extern HANDLE ghPlayerQuitEvent;


// Player Handling
struct PlayerData {	//Ta struktura jest do petli w grze
	int ID;				//Id gracza 
	char* nickName;		//jego nickname opcjonalnie moge to zaimplementowac 
	char lastMove;		//ostatni ruch graczas
}typedef PlayerData;

struct PlayerServerInfo {//Ta natomiast do obslugiwania gracza przez serwer
	PlayerData * data;
	SOCKET * socket;
	HANDLE * receivingThread;
	HANDLE * sendingThread;
	struct sockaddr_in * sockAddr;
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

HANDLE* startServer(int* PORT, int maxPlayers);		//Przygotowuje watek serwera i wywoluje go
DWORD WINAPI serverThread(void* data);				//Glowny Watek serwera


//Threads Handling
HANDLE* startReceivingThreads(PlayerServerInfo** players, int connected);	//Rozpocznya watki do odbierania danych
HANDLE* startSendingThreads(PlayerServerInfo** players, int connected);		//Rozpoczyna watki do wysylania danych
DWORD WINAPI receiveDataFromPlayer(void * data);							//Watek otrzymywania danych - data powinna wskazywac
DWORD WINAPI sendingDataToPlayer(void* data);								//na struct PlayerServerInfo *

