#include "IncludingList.h"
#include "server.h"
#include "player.h"

#pragma comment(lib, "Ws2_32.lib")

HANDLE ghStopEvent;

int main(int argc, char* argv[])
{

	//Read data from command line
	char * ADRESS = "192.168.1.2";
	int maxPlayers = 2;
	int PORT = 5037;
	_Bool isHost = TRUE;
	//Start win api service	
	WSADATA wsas;
	WORD wersja;
	wersja = MAKEWORD(2, 0);
	WSAStartup(wersja, &wsas);
	ghStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (isHost){
		//Start server in a new thread
		HANDLE * serverThread = startServer(&PORT, maxPlayers);
	}

	//Get server data from input scanf(..)
	//TODO
	if (startPlayerThreads(&PORT, ADRESS, isHost) < 0)
	{
		
	}
	SetEvent(ghStopEvent);

	if (isHost)
	{
		WaitForSingleObject(serverThread, INFINITE);
		CloseHandle(*serverThread);
		free(serverThread);
	}
	CloseHandle(ghStopEvent);
	return 0;
}

