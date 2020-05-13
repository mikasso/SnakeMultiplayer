#include "server.h"
#define MAX_PLAYERS  1
char gMove[MAX_PLAYERS];

int startServer(HANDLE* serverHandle, int * PORT) {
	//Create server in a new therad
	DWORD serverThreadID;
	*serverHandle = CreateThread(
		NULL, // atrybuty bezpieczenstwa
		0,	  // inicjalna wielkosc stosu
		serverThread, // funkcja watku
		PORT,// dane dla funkcji watku
		0, // flagi utworzenia
		&serverThreadID);

	if (*serverHandle == INVALID_HANDLE_VALUE)
	{
		printf("Server thread can not have been created! id =  %x \n", serverThreadID);
		WSACleanup();
		exit(-1);
		return -1;
	}
	return 1;
}


DWORD WINAPI serverThread(int* PORT)
{
	//Creating server socket
	SOCKET serverSocket;
	struct sockaddr_in sa;

	serverSocket  = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(*PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(serverSocket, (struct sockaddr FAR*) & sa, sizeof(sa)) != SOCKET_ERROR)
	{
		listen(serverSocket, 1);
		SOCKET connectionSocket [MAX_PLAYERS] ;
		HANDLE connectionsToPlayers[MAX_PLAYERS];
		int playersNumber = 0;
		struct sockaddr_in sc;
		int lenc;
		lenc = sizeof(sc);
		//Waiting for connection
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			playersNumber++;
			connectionSocket[i] = accept(serverSocket, (struct sockaddr FAR*) & sc, &lenc);

			PlayerData * data = (PlayerData*)malloc(sizeof(PlayerData));
			data->ID = i;
			data->socket = &connectionSocket[i];

			DWORD threadID;
			connectionsToPlayers[i] = CreateThread(
				NULL, // atrybuty bezpieczenstwa
				0,	  // inicjalna wielkosc stosu
				receiveDataFromPlayers, // funkcja watku
				&data,// dane dla funkcji watku
				0, // flagi utworzenia
				&threadID);

			if (connectionsToPlayers[i] == INVALID_HANDLE_VALUE)
			{
				closesocket(connectionSocket[i]);
				printf("connectionToPlayer thread can not have been created! id =  %x \n", threadID);
				return -(i + 1);
			}
		}

			//sending to each connection data in thread
			for (int i = 0; i < playersNumber; i++)
			{
				char buf[80];
				int dlug;
				//If event was called than stop sending messages.
				while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT)
				{
					fgets(buf, 80, stdin);
					dlug = strlen(buf);
					buf[dlug - 1] = '\0';
					send(connectionSocket[i], buf, dlug, 0);
					if (strcmp(buf, "KONIEC") == 0)
					{
						break;
					}
				}
			}

		
	}
	//End thread job
	SetEvent(ghStopEvent);
	printf("Turning off server.");
	ExitThread(0);
	return 0;
}



void receiveDataFromPlayers(void * data)
{
	PlayerData * playerData = (PlayerData *) data;
	char buf[80]; //buffor to read msgs
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT) {
		if (recv(playerData->socket, buf, 80, 0) > 0)
		{
			if (strcmp(buf, "KONIEC") == 0)
			{
				//Stop loop
				return;
			}
			//Redirect it to gameLoop
			gMove[playerData->ID] = buf[0];

		}
	}
	closesocket(*connectionSocket);
}