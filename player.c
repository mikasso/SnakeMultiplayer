#include "player.h"

int startPlayerThreads(int* PORT,char * ADRESS, _Bool isHost) {
	HANDLE sendClientInputHandle,  gameViewerHandle;
	ghPlayerQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ghGameEndedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	//Przygotowanie gniazda i struktury adresu
	SOCKET clientSocket;
	struct sockaddr_in sa;
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(*PORT);
	sa.sin_addr.s_addr = inet_addr(ADRESS);
	//Polaczenie sie z sewerem
	int attemp = 0;
	int result = SOCKET_ERROR;
	while (result == SOCKET_ERROR)
	{
		printf("Attemp to connect server.\nIP: %s \nPort: %d\n", ADRESS, *PORT);
		result = connect(clientSocket, (struct sockaddr FAR*) & sa, sizeof(sa));
		if (result == SOCKET_ERROR)
		{
			printf("Connection error!\n %d",WSAGetLastError());
		}
		attemp += 1;
		if (attemp == ATTEMPS_LIMIT)
		{
			printf("Error: reached limit attemps to connect server!\n");
			return -1;
		}
	}
	//Stworzenie watku odpowiedzialnego za pobieranie danych z klawiatury i wysylyania ich 
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
	//Stworzenie watku pobierajacego dane z serwera i wyswietlajacego go
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
	//Oczekiwanie na zakonczenie obu watkow
	WaitForSingleObject(gameViewerHandle, INFINITE);
	WaitForSingleObject(sendClientInputHandle, INFINITE);

	if (isHost)
		SetEvent(ghStopEvent);

	CloseHandle(gameViewerHandle);
	CloseHandle(sendClientInputHandle);
	closesocket(clientSocket);
	return 0;
}



DWORD WINAPI sendClientInput(void * clientSocket)
{
	SOCKET* socket = clientSocket;
	char c;
	//If event was called than stop sending messages.
	while (WaitForSingleObject(ghGameEndedEvent, 1) == WAIT_TIMEOUT)
	{
		c = _getch();
		if (c == QUIT_KEY)
		{
			SetEvent(ghPlayerQuitEvent);
			break;
		}
		send(*socket, &c, sizeof(c), 0);
	}
	return 0;
}