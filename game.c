#include "game.h"


PlayerShowData ** getNamedPlayers(char * nicks,int * count)
{
	*count = (int) *nicks;
	int c = *count;
	PlayerShowData** players = malloc(sizeof(PlayerShowData*) * c);
	char* from = &nicks[4];
	int start = 0,end = 0;
	for (int i = 0; i < c; i++)
	{
		players[i] = malloc(sizeof(PlayerShowData));
		while (from[end] != '\0')
		{
			end++;
		}
		end++;
		int len = end - start;
		players[i]->nickName = malloc(sizeof(len));
		memcpy(players[i]->nickName, &from[start], len);
		gotoxy(XSIZE + 10 + i, 1);
		puts(players[i]->nickName);
		start = end;
	}
	return players;
}


DWORD WINAPI  viewGameBoard(void* socket)
{
	SOCKET* connectionSocket = (SOCKET*)socket;
	hidecursor();
	char buf[XSIZE+1]; //buffor to read msgs
	buf[XSIZE] = '\0';
	int count, y = 0;
	char nicks[NICKS_LEN];
	recv(*connectionSocket, nicks, sizeof(nicks), 0);
	PlayerShowData ** players = getNamedPlayers(nicks, &count);
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghPlayerQuitEvent, 1) == WAIT_TIMEOUT) {
		if (recv(*connectionSocket, buf, XSIZE, 0) > 0)
		{
			if (strcmp(buf, "KONIEC") == 0)
			{
				SetEvent(ghGameEndedEvent);
				return 0;
			}
			gotoxy(1, y);
			puts( buf);
			y = (y + 1) % YSIZE;
		}
	}
	//free other players informations
	for (int i = 0; i < count; i++)
	{
		free(players[i]->nickName);
		free(players[i]);
	}
	free(players);
	return 0;
}

DWORD WINAPI gameLoop(void * data)
{
	//Odczyt danych
	GameData * gameData = ( GameData * ) data;
	PlayerData** players = malloc(sizeof(PlayerData));
	BINARY_SEMAPHORE(&ghBoardSemaphore);

	int gameStatus = ON;
	int count = gameData->count;

	for (int i = 0; i < count; i++)
	{
		players[i] = gameData->players[i]->data;
		players[i]->alive = TRUE;
		players[i]->lastMove = '\0';
	}


	char board[YSIZE][XSIZE];
	char c;
	int t = 1000;
	_Bool toggle = FALSE;
	while (gameStatus == ON)
	{
		//AKCJA gry

		for (int i = 0; i < count; i++)
		{
			//SYNCHRONZIACJA
			LOCK(&players[i]->playerSemaphore);
			c = players[i]->lastMove;				//Pobranie ruchu kazdego gracza
			UNLOCK(&players[i]->playerSemaphore);
			//KONIEC
			ResetEvent(ghPlayersReceivedEvent[i]);
			//translacja wê¿a o ruch c
		}



		//WPISANIE kazdego weza na mape

		if(toggle)
			memset(board, '_', BOARD_SIZE * sizeof(char));
		else
			memset(board, '|', BOARD_SIZE * sizeof(char));
		

		//SYNCHRONIZACJA
		LOCK(&ghBoardSemaphore);
		memcpy(BOARD, board, BOARD_SIZE);
		UNLOCK(&ghBoardSemaphore);
		//KONIEC

		toggle = !toggle;
		if (t-- == 0)
			break;
		Sleep(10);
		//czekaj az wszyscy gracze odbiora stan gry
		WaitForMultipleObjects(count, ghPlayersReceivedEvent, 1, INFINITE);
	}

	free(players);
	CloseHandle(ghBoardSemaphore);
	SetEvent(ghGameEndedEvent);
	return 0;
}

void gotoxy(int x, int y)
{
	COORD c;
	c.X = x - 1;
	c.Y = y - 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

void hidecursor()
{
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}

void BINARY_SEMAPHORE(HANDLE* sempahore) 
{
*sempahore = CreateSemaphore(
		NULL,           // default security attributes
		1,				// initial count
		1,				// maximum count
		NULL);          // unnamed semaphore
}

DWORD LOCK(HANDLE * sempahore)
{
	DWORD dwWaitResult = WaitForSingleObject(
		*sempahore,   // handle to semaphore
		INFINITE);           // zero-second time-out interval
	return dwWaitResult;
}

_Bool UNLOCK(HANDLE* sempahore)
{
	return ReleaseSemaphore(
		*sempahore,  // handle to semaphore
		1,            // increase count by one
		NULL);
}