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

void showPlayerInfo(PlayerServerInfo* player) {
	struct sockaddr_in* name = malloc(sizeof(struct sockaddr_in));
	int sizeSockAddr = sizeof(struct sockaddr);
	getpeername(*player->socket, (struct sockaddr*) name, &sizeSockAddr);
	printf("Connected with player IP: %s \t ID: %d \n", inet_ntoa(name->sin_addr), player->data->ID);
	free(name);
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
			players[i]->status = CONNECTED;
			players[i]->othersServerInfo = players;
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
	for(int i=0;i<connected;i++)
		players[i]->count = connected;
	//Prepare players to start the game
	displayStartInfo();
	//Start all player threads
	nickNamesReceived = malloc(sizeof(HANDLE) * connected);
	ghPlayersReceivedEvent = malloc(sizeof(HANDLE) * connected);
	for (int i = 0; i < connected; i++)
	{
		nickNamesReceived[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
		ghPlayersReceivedEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	HANDLE * receivingDataThreads = startReceivingThreads(players, connected);

	WaitForMultipleObjects(connected, nickNamesReceived, 1, INFINITE);

	HANDLE gameLoopThread = NULL;
	GameData gameData;
	gameData.players = players;
	gameData.count = connected;
	ghGameReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	runThread(&gameLoopThread, &gameLoop, &gameData);
	WaitForSingleObject(ghGameReady, INFINITE);
	HANDLE* sendingDataThreads = startSendingThreads(players, connected);
	
	WaitForSingleObject(gameLoopThread, INFINITE);
	WaitForMultipleObjects(connected, sendingDataThreads, 1, INFINITE);
	WaitForMultipleObjects(connected, receivingDataThreads, 1, INFINITE);
	//End thread job
	for (int i = 0; i < connected; i++)
	{
		CloseHandle(nickNamesReceived[i]);
		CloseHandle(sendingDataThreads[i]); 
		CloseHandle(receivingDataThreads[i]);
		freePlayerServerInfo(players[i]);
	}
	CloseHandle(gameLoopThread);
	printf("Turning off server.");
	return 0;
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
	char c,buf[20];
	int len, code, result;
	//Read nicknames of each player
	result = recv(*playerServerData->socket, buf, sizeof(buf), 0);
	len = strlen(buf)+1;
	playerData->nickName = malloc(len);
	memcpy(playerData->nickName, buf,len);
	SetEvent(nickNamesReceived[playerData->ID]);
	//non blokcing socket 

	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT ) {
		result = recv(*playerServerData->socket, &c, sizeof(c), 0);
		if(result > 0)
		{
			//SYNCHRONIZACJA
			LOCK(&playerData->playerSemaphore);
			playerData->lastMove = c;
			UNLOCK(&playerData->playerSemaphore);
			//KONIEC
			if (c == QUIT_KEY)
			{
				//Stop loop
				break;
			}
			//printf("server received from player ID = %d key %c", playerData->ID, c);
		}else
			break;
	}
	gotoxy(1, YSIZE+5);
	printf("Player %d disconnected \n", playerData->ID);
	playerServerData->status = DISCONNECTED;
	return 0;
}

DWORD WINAPI sendingDataToPlayer(void * data)
{
	PlayerServerInfo* playerServerData = (PlayerServerInfo*) data;			//informacje o graczu dla ktorego dedydkowane jest ten watek
	PlayerData* playerData = playerServerData->data;						// to samo co u gory tylko wypakowane
	//PlayerShowData temp;													// struktura do wypelniania do przesylu zapakowanych danych
	PlayerServerInfo ** others = playerServerData->othersServerInfo;		// tablica informacji o innych graczach
	SOCKET * connectionToPlayer = playerServerData->socket;			
	char buf[4000], nicksInfo[NICKS_LEN], * nick = NULL;						// tablica na bufor nicki wszystkich graczy i wskaznik na konkrertny
	int count = playerServerData->count;
	nicksInfo[0] =(int)count;
	int len,ptr = sizeof(playerServerData->count);
	for (int i = 0; i < playerServerData->count; i++)
	{
		nick = others[i]->data->nickName;
		len = strlen(nick) + 1;
		memcpy(&nicksInfo[ptr], nick, len);
		ptr += len;
	}
	int result = send(*connectionToPlayer, nicksInfo, sizeof(nicksInfo), 0);
	
	while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT && playerServerData->status == CONNECTED )
	{
		WaitForSingleObject(ghLoopDone, INFINITE);
		//SYNCHRONIZACJA
		if (gameStatus == OFF)
			break;
			int ptr = 0;
			for (int i = 0; i < count; i++)
			{
				//LOCK(&others[i]->data->playerSemaphore);
				buf[ptr++] = (char)others[i]->data->ID;
				if (others[i]->data->ID < 0)
				{
					int c = 4;
					c++;
				}
				buf[ptr++] = (char)others[i]->data->color;
				buf[ptr++] = (char)others[i]->data->score;
				buf[ptr++] = (char)others[i]->data->alive;
				if (others[i]->data->alive == TRUE)
				{

					buf[ptr++] = (char)others[i]->data->size;
					for (int j = 0; j < others[i]->data->size; j++)
					{
						buf[ptr++] = (char)others[i]->data->x[j];
						buf[ptr++] = (char)others[i]->data->y[j];
					}
				}
				//UNLOCK(&others[i]->data->playerSemaphore);
			}
			for (int i = 0; i < APPLES; i++)
			{
				buf[ptr++] = (char)gApples[i].alive;
				buf[ptr++] = (char)gApples[i].x;
				buf[ptr++] = (char)gApples[i].y;
			}
			result = send(*connectionToPlayer, buf, ptr, 0);
			if (result == 0)
				break;
			SetEvent(ghPlayersReceivedEvent[playerData->ID]);
	}
	SetEvent(ghPlayersReceivedEvent[playerData->ID]);
	closesocket(*connectionToPlayer);
	return 0;
}

PlayerServerInfo* initPlayerServerInfo()
{
	PlayerServerInfo* info = malloc(sizeof(PlayerServerInfo));
	info->status = DISCONNECTED;
	info->socket = malloc(sizeof(SOCKET));
	info->receivingThread = malloc(sizeof(HANDLE));
	info->sendingThread = malloc(sizeof(HANDLE));
	info->data = malloc(sizeof(PlayerData));
	info->data->nickName = NULL;
	BINARY_SEMAPHORE(&info->data->playerSemaphore);
	info->sockAddr = malloc(sizeof(struct sockaddr_in));
	return info;
}

void freePlayerServerInfo(PlayerServerInfo* info)
{
	closesocket(*info->socket);
	free(info->sockAddr);
	CloseHandle(info->data->playerSemaphore);
	if (info->data->nickName != NULL)
		free(info->data->nickName);
	free(info->data);
	free(info->socket);
	free(info);
}