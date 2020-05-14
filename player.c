#include "player.h"

int startPlayerThreads(int* PORT,char * ADRESS, _Bool isHost) {
	HANDLE sendClientInputHandle,  gameViewerHandle;

	//Prepare client socket with ip and port 
	SOCKET clientSocket;
	struct sockaddr_in sa;
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(*PORT);
	sa.sin_addr.s_addr = inet_addr(ADRESS);
	ghPlayerQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ghGameEndedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	int attemp = 0;
	while (attempToConnect(&clientSocket, &sa, ADRESS, PORT) == FALSE)
	{
		attemp += 1;
		if (attemp == ATTEMPS_LIMIT)
		{
			printf("Error: reached limit attemps to connect server!\n");
			return -1;
		}
	}

	//Create server in a new therad
	DWORD sendClientInputThreadID;
	sendClientInputHandle = CreateThread(
		NULL, // atrybuty bezpieczenstwa
		0,	  // inicjalna wielkosc stosu
		sendClientInput, // funkcja watku
		&clientSocket,// dane dla funkcji watku
		0, // flagi utworzenia
		&sendClientInputThreadID);

	if (sendClientInputHandle == INVALID_HANDLE_VALUE)
	{
		closesocket(clientSocket);
		printf("sendClientInputthread can not have been created! id =  %x \n", sendClientInputThreadID);
		return -2;
	}

	DWORD gameViewerThreadID;
	gameViewerHandle = CreateThread(
		NULL, // atrybuty bezpieczenstwa
		0,	  // inicjalna wielkosc stosu
		viewGame, // funkcja watku
		&clientSocket,// dane dla funkcji watku
		0, // flagi utworzenia
		&gameViewerThreadID);

	if (gameViewerHandle == INVALID_HANDLE_VALUE)
	{
		SetEvent(ghStopEvent);
		WaitForSingleObject(sendClientInputHandle, INFINITE);
		CloseHandle(sendClientInputHandle);
		closesocket(clientSocket);
		printf("gameViewer thread can not have been created! id =  %x \n", gameViewerThreadID);
		return -3;
	}

	WaitForSingleObject(gameViewerHandle, INFINITE);
	WaitForSingleObject(sendClientInputHandle, INFINITE);

	if (isHost)
		SetEvent(ghStopEvent);

	CloseHandle(gameViewerHandle);
	CloseHandle(sendClientInputHandle);
	closesocket(clientSocket);

	return 0;
}

_Bool attempToConnect(SOCKET* clientSocket, struct sockaddr_in* sa, char* ADRESS, int* PORT)
{
	printf("Attemp to connect server.\nIP: %s \nPort: %d\n", ADRESS, *PORT);
	int result;
	{
		result = connect(*clientSocket, (struct sockaddr FAR*) sa, sizeof(*sa));
		if (result == SOCKET_ERROR)
		{
			printf("Connection error!\n");
			return FALSE;
		}
	} 
	printf("\n\nConnected succesfully!\n");
	printf("---------------------------------------------\n\n");
	return TRUE;
}

DWORD WINAPI sendClientInput(void * clientSocket)
{
	SOCKET* socket = clientSocket;
	char c;
	//If event was called than stop sending messages.
	while (WaitForSingleObject(ghGameEndedEvent, 1) == WAIT_TIMEOUT)
	{
		c = _getch();
		send(*socket, &c, sizeof(c), 0);
		if (c == QUIT_KEY)
		{
			SetEvent(ghPlayerQuitEvent);
			break;
		}
	}
	return 0;
}