#include "player.h"



_Bool runThread(HANDLE * handler,ThreadFunc fun, void * data)
{
	DWORD threadID;
	*handler =  CreateThread(
		NULL, // atrybuty bezpieczenstwa
		0,	  // inicjalna wielkosc stosu
		fun, // funkcja watku
		data,// dane dla funkcji watku
		0, // flagi utworzenia
		&threadID);
	if (*handler == INVALID_HANDLE_VALUE)
	{
		printf("Thread couldn't start \n %d",threadID);
		return FALSE;
	}
	return TRUE;
}

int startPlayerThreads(int* PORT,char * ADDRESS, _Bool isHost) {
	HANDLE sendClientInputHandle,  gameViewerHandle;
	ghPlayerQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ghGameEndedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	SOCKET clientSocket;
	struct sockaddr_in sa;
	if (connectToServer(&clientSocket, &sa, PORT, ADDRESS) == FALSE)
		return -1;

	//Stworzenie watku odpowiedzialnego za pobieranie danych z klawiatury i wysylyania ich 
	runThread(&sendClientInputHandle, sendClientInput, &clientSocket);

	//Stworzenie watku pobierajacego dane z serwera i wyswietlajacego go
	runThread(&gameViewerHandle, viewGame, &clientSocket);
	if (gameViewerHandle == INVALID_HANDLE_VALUE)
	{
		SetEvent(ghStopEvent);
		WaitForSingleObject(sendClientInputHandle, INFINITE);
		CloseHandle(sendClientInputHandle);
		closesocket(clientSocket);
		return -1;
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

_Bool connectToServer(SOCKET * clientSocket, struct sockaddr_in  * sa, int * PORT, char * ADDRESS)
{
	//Przygotowanie gniazda i struktury adresu
	*clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)(sa), 0, sizeof(sa));
	sa->sin_family = AF_INET;
	sa->sin_port = htons(*PORT);
	sa->sin_addr.s_addr = inet_addr(ADDRESS);
	//Polaczenie sie z sewerem
	int attemp = 0;
	int result = SOCKET_ERROR;
	while (result == SOCKET_ERROR)
	{
		printf("Attemp to connect server.\nIP: %s \nPort: %d\n", ADDRESS, *PORT);
		result = connect(*clientSocket, (struct sockaddr FAR*) sa, sizeof(*sa));
		if (result == SOCKET_ERROR)
		{
			printf("Connection error!\n %d", WSAGetLastError());
		}
		attemp += 1;
		if (attemp == ATTEMPS_LIMIT)
		{
			printf("Error: reached limit attemps to connect server!\n");
			return FALSE;
		}
	}
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
		if (c == QUIT_KEY)
		{
			SetEvent(ghPlayerQuitEvent);
			break;
		}
		send(*socket, &c, sizeof(c), 0);
	}
	return 0;
}
