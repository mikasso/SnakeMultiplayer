#include "IncludingList.h"
#include "server.h"
#include "player.h"

#pragma comment(lib, "Ws2_32.lib")

extern HANDLE ghGameEndedEvent;
extern HANDLE ghStopEvent;
extern HANDLE ghPlayerQuitEvent;

int main(int argc, char* argv[])
{

	//Read data from command line
	char * ADRESS = "127.1.1.1";
	char* nick = "Mikas";
	int maxPlayers = 2;
	int PORT = 5037;
	_Bool isHost = TRUE;
	//Start win api service	
	WSADATA wsas;
	WORD wersja;
	wersja = MAKEWORD(2, 0);
	WSAStartup(wersja, &wsas);
	ghStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HANDLE server = NULL;
	ServerBasicData * serverData = NULL;
	if (isHost){
		//Creating server init information
		serverData = malloc(sizeof(ServerBasicData));
		//Filing serverData structure
		serverData->socket = socket(AF_INET, SOCK_STREAM, 0);
		memset((void*)(&serverData->addr_in), 0, sizeof(serverData->addr_in));
		serverData->addr_in.sin_family = AF_INET;
		serverData->addr_in.sin_port = htons(PORT);
		serverData->addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
		serverData->maxPlayers = maxPlayers;
		//Create the server in a new therad
		runThread(&server, serverThread, serverData);
	}

	//Get server data from input scanf(..)
	//TODO
	if (startPlayerThreads(&PORT, ADRESS, nick, isHost) < 0)
	{
		printf("Client cannot reach server! \n");
	}
	SetEvent(ghStopEvent);
	if (isHost)
	{
		printf("\n%d",WaitForSingleObject(server, INFINITE));
		free(serverData);
		CloseHandle(server);
	}
	CloseHandle(ghStopEvent);
	return 0;
}

