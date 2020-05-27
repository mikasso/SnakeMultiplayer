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

	int playersNumber = 0;
	SOCKET connectionSocket[MAX_PLAYERS];
	HANDLE receivingDataThreads[MAX_PLAYERS];
	HANDLE sendingDataThreads[MAX_PLAYERS];
	struct sockaddr_in clientSockAddr[MAX_PLAYERS];
	PlayerData* playerData[MAX_PLAYERS];
	int result = bind(serverSocket, (struct sockaddr FAR*) & sa, sizeof(sa));
	if (result != SOCKET_ERROR)
	{
		listen(serverSocket, MAX_PLAYERS);			//Set how many connections [players] can be at one time
		int lenc;
		//Waiting for connection
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			lenc = sizeof(clientSockAddr[i]);
			playersNumber++;
			connectionSocket[i] = accept(serverSocket, (struct sockaddr FAR*) & clientSockAddr[i], &lenc);

			playerData[i] = (PlayerData*)malloc(sizeof(PlayerData));
			playerData[i]->ID = i;
			playerData[i]->socket = &connectionSocket[i];

			DWORD threadID;
			receivingDataThreads[i] = CreateThread(
				NULL, // atrybuty bezpieczenstwa
				0,	  // inicjalna wielkosc stosu
				receiveDataFromPlayer, // funkcja watku
				playerData[i],// dane dla funkcji watku
				0, // flagi utworzenia
				&threadID);

			if (receivingDataThreads[i] == INVALID_HANDLE_VALUE)
			{
				printf("connectionToPlayer thread can not have been created! id =  %x \n", threadID);
				return -(i + 1);
			}

		

			sendingDataThreads[i] = CreateThread(
				NULL, // atrybuty bezpieczenstwa
				0,	  // inicjalna wielkosc stosu
				sendingDataToPlayer, // funkcja watku
				playerData[i]->socket,// dane dla funkcji watku
				0, // flagi utworzenia
				& threadID);
		}		
	}
	else
	{
		printf("Socket error %d \n" ,result);
		Sleep(1000);
	}
	system("CLS");
	WaitForMultipleObjects(playersNumber, sendingDataThreads, 1, INFINITE);
	WaitForMultipleObjects(playersNumber, receivingDataThreads, 1, INFINITE);

	//End thread job
	for (int i = 0; i < playersNumber; i++)
	{
		CloseHandle(sendingDataThreads[i]);
		CloseHandle(receivingDataThreads[i]);
		closesocket(*playerData[i]->socket);
		free(playerData[i]);
	}
	SetEvent(ghStopEvent);
	printf("Turning off server.");
	ExitThread(0);
	return 0;
}


DWORD WINAPI receiveDataFromPlayer(void * data)
{
	PlayerData * playerData = (PlayerData *) data;
	char c; //buffor to read msgs
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT) {
		if (recv(* playerData->socket, &c, sizeof(c), 0) > 0)
		{
			if (c == QUIT_KEY)
			{
				//Stop loop
				return 0;
			}
			//Redirect recieved data to gameLoop TODO
			gotoxy(1, 3);
			printf("server received from player ID = %d key %c", playerData->ID, c);
			gMove[playerData->ID] = c;

		}
	}
	return 0;
}

DWORD WINAPI sendingDataToPlayer(void * data)
{
	SOCKET * clientSocket = (SOCKET *) data;
	int dlug;
	char buf[BOARD_SIZE];
	_Bool ping = 1;
	while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT)
	{
		//Here getting messeges about game status from game thread
		//TODO

		//Send data
		Sleep(1000);
		if(ping)
			strcpy_s(buf,5, "ping");
		else
			strcpy_s(buf,5, "pong");
		ping = !ping;
		dlug = strlen(buf);
		buf[dlug] = '\0';
		send(*clientSocket, buf, dlug + 1, 0);
		if (strcmp(buf, "KONIEC") == 0)
		{
			break;
		}
	}
	return 0;
}