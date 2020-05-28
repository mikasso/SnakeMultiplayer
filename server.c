#include "server.h"
#define WAIT_TO_START_TIME 0

void displayStartInfo() {
	system("CLS");
	for (int i = WAIT_TO_START_TIME; i > 0; i--)
	{
		printf("Game will start in %ds\n",i);
		Sleep(1000);
	}
	system("CLS");
}

HANDLE * startServer(int * PORT, int maxPlayers) {
	HANDLE * thread = malloc(sizeof(thread));
	//Creating server init information
	ServerBasicData * serverData = malloc(sizeof(ServerBasicData));
	//Filing serverData structure
	serverData->socket = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)(& serverData->addr_in), 0, sizeof(serverData->addr_in));
	serverData->addr_in.sin_family = AF_INET;
	serverData->addr_in.sin_port = htons(*PORT);
	serverData->addr_in.sin_addr.s_addr = htonl(INADDR_ANY);
	serverData->maxPlayers = maxPlayers;
	//Create the server in a new therad
	runThread(thread, serverThread, serverData);

	if (*thread == INVALID_HANDLE_VALUE)
	{
		free(&serverData->addr_in);
		free(&serverData->socket);
		free(serverData);
		return NULL;
	}
	return *thread;
}


DWORD WINAPI serverThread(void * data)
{
	//Unpack data and prepare to work
	ServerBasicData * serverData = data;
	int connected = 0;
	int maxPlayers = serverData->maxPlayers;
	PlayerServerInfo ** players = malloc(sizeof(PlayerServerInfo *) * maxPlayers);
	//Bind socket server
	int result = bind(serverData->socket, (struct sockaddr FAR*) & serverData->addr_in, sizeof(serverData->addr_in));
	if (result != SOCKET_ERROR)
	{
		listen(serverData->socket, maxPlayers);			//Set how many connections [players] can be at one time
		int lenc;
		for (int i = 0; i < maxPlayers; i++)
		{
			//Waiting for connection
			players[i] = initPlayerServerInfo();
			players[i]->data->ID = i;
			lenc = sizeof(*players[i]->sockAddr);
			*players[i]->socket = accept(serverData->socket, (struct sockaddr FAR*) players[i]->sockAddr, &lenc);
			connected++;
			showPlayerInfo(players[i]);
			printf("Free slot left:%d", maxPlayers - connected);
		}		
	}
	else
	{
		printf("Socket error %d \n" ,result);
		Sleep(1000);
	}
	//Prepare players to start the game
	displayStartInfo();
	//Start all player threads
	HANDLE * sendingDataThreads = startSendingThreads(players, connected);
	HANDLE * receivingDataThreads = startReceivingThreads(players, connected);
	/*
		Teraz tak w tym miejscu powinno byc odpalenie watku zawierajacego petle gry
		Watek ten powinien w magiczny sposob odczytywac dane ktore sa odbierane w watkach 
		receiveDataFromPlayer oraz udostepniac je watka sendingDataToPlayer
	*/

	//Wait them to finish
	WaitForMultipleObjects(connected, sendingDataThreads, 1, INFINITE);
	WaitForMultipleObjects(connected, receivingDataThreads, 1, INFINITE);
	//End thread job
	SetEvent(ghStopEvent);
	for (int i = 0; i < connected; i++)
	{
		freePlayerServerInfo(players[i]);
	}
	printf("Turning off server.");
	return 0;
}

void showPlayerInfo(PlayerServerInfo * player) {
	struct sockaddr_in* name = malloc(sizeof(struct sockaddr_in));
	int sizeSockAddr = sizeof(struct sockaddr);
	getpeername(*player->socket,(struct sockaddr *) name, &sizeSockAddr);
	printf("Connected with player IP: %s \t ID: %d \n", inet_ntoa(name->sin_addr),player->data->ID);
	free(name);
}

HANDLE * startReceivingThreads(PlayerServerInfo ** players, int connected)
{
	HANDLE * threads = malloc(sizeof(HANDLE) * connected);
	for (int i = 0; i < connected; i++) {
		runThread(&threads[i], receiveDataFromPlayer, players[i]);
		if (threads[i] == INVALID_HANDLE_VALUE)
		{
			printf("receiveDataFromPlayer thread can not have been created! \n");
			return threads;
		}
		//przypisanie do struktury gracza watku odpowiedzialnego za otrzymywanie danych
		players[i]->receivingThread = &threads[i];
	}
	return threads;
}

HANDLE* startSendingThreads(PlayerServerInfo** players, int connected)
{
	HANDLE* threads = malloc(sizeof(HANDLE) * connected);
	for (int i = 0; i < connected; i++) {
		runThread(&threads[i], sendingDataToPlayer, players[i]);
		if (threads[i] == INVALID_HANDLE_VALUE)
		{
			printf("sendingDataToPlayer thread can not have been created! \n");
			return threads;
		}
		//przypisanie do struktury gracza watku odpowiedzialnego za wysylanie danych
		players[i]->sendingThread = &threads[i];
	}
	return threads;
}

DWORD WINAPI receiveDataFromPlayer(void * data)
{
	PlayerServerInfo * playerServerData = (PlayerServerInfo *) data;
	PlayerData * playerData = playerServerData->data;
	char c; //buffor to read msgs
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT) {
		if (recv(*playerServerData->socket, &c, sizeof(c), 0) > 0)
		{
			if (c == QUIT_KEY)
			{
				//Stop loop
				return 0;
			}
			//Redirect recieved data to gameLoop TODO
			gotoxy(1, 3);
			printf("server received from player ID = %d key %c", playerData->ID, c);
			//Jakos przeslijcie te dane to watku gry
		}
	}
	return 0;
}

DWORD WINAPI sendingDataToPlayer(void * data)
{
	PlayerServerInfo* playerServerData = (PlayerServerInfo*) data;
	PlayerData* playerData = playerServerData->data;
	SOCKET * connectionToPlayer = playerServerData->socket;
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
		send(*connectionToPlayer, buf, dlug + 1, 0);
		if (strcmp(buf, "KONIEC") == 0)
		{
			break;
		}
	}
	return 0;
}

PlayerServerInfo* initPlayerServerInfo()
{
	PlayerServerInfo* info = malloc(sizeof(PlayerServerInfo));
	info->socket = malloc(sizeof(SOCKET));
	info->receivingThread = malloc(sizeof(HANDLE));
	info->sendingThread = malloc(sizeof(HANDLE));
	info->data = malloc(sizeof(PlayerData));
	info->sockAddr = malloc(sizeof(struct sockaddr_in));
	return info;
}

void freePlayerServerInfo(PlayerServerInfo* info)
{
	closesocket(*info->socket);
	CloseHandle(*info->receivingThread);
	CloseHandle(*info->sendingThread);
	free(info->sockAddr);
	free(info->data);
	free(info->sendingThread);
	free(info->receivingThread);
	free(info->socket);
	free(info);
}