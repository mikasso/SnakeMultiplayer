#include "game.h"

DWORD WINAPI viewGame(void * connectionSocket)
{
	//Creating server socket
	SOCKET * connectionToServer = (SOCKET *) connectionSocket;
	viewGameBoard(connectionToServer);

	ExitThread(0);
}

void viewGameBoard(SOCKET* connectionSocket)
{
	hidecursor();
	char buf[XSIZE+1]; //buffor to read msgs
	buf[XSIZE] = '\0';
	int y = 0;
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghPlayerQuitEvent, 1) == WAIT_TIMEOUT) {
		if (recv(*connectionSocket, buf, XSIZE, 0) > 0)
		{
			if (strcmp(buf, "KONIEC") == 0)
			{
				SetEvent(ghGameEndedEvent);
				return;
			}
			gotoxy(1, y);
			puts( buf);
			y = (y + 1) % YSIZE;
		}
	}
	return;
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
		players[i]->lastMove = NULL;
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
		Sleep(400);
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