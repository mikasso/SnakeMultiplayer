#include "IncludingList.h"
#include "server.h"
#include "player.h"

#pragma comment(lib, "Ws2_32.lib")

HANDLE ghStopEvent;

int main(int argc, char* argv[])
{
	//Read data from command line
	char* ADRESS = "127.0.0.1";
	int PORT = 2137;
	_Bool isHost = TRUE;
	//Start win api service	
	WSADATA wsas;
	WORD wersja;
	wersja = MAKEWORD(2, 0);
	WSAStartup(wersja, &wsas);
	ghStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (isHost)
		//Start server in a new thread
	{
		HANDLE serverThread;
		startServer(&serverThread, &PORT);
	}
	//Get server data to connect with it
	//TODO

	if (startPlayerThreads(&PORT, ADRESS, isHost) < 0)
	{
		
	}


	SetEvent(ghStopEvent);
	WaitForSingleObject(serverThread, INFINITE);
	CloseHandle(serverThread);
	CloseHandle(ghStopEvent);

	return 0;
}

