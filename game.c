#include "game.h"
#define MINTIME 200

void drawBorders() {
	for (int i = 0; i < YSIZE; i++)
	{
		gotoxy(XSIZE, i+1);
		puts("|");
	}
	for (int i = 0; i < XSIZE; i++)
	{
		gotoxy(1+i, YSIZE);
		puts("_");
	}
	for (int i = 0; i < YSIZE; i++)
	{
		gotoxy(1, i + 1);
		puts("|");
	}
	for (int i = 0; i < XSIZE; i++)
	{
		gotoxy(1 + i, 1);
		puts("_");
	}
	gotoxy(XSIZE, YSIZE);
	puts("/");
	gotoxy(1, 1);
	puts("/");
}

void drawApple(Apple * a, HANDLE* hConsole)
{
		SetConsoleTextAttribute(*hConsole, FOREGROUND_GREEN);
			gotoxy(a->x, a->y);
			puts("@");

}

void drawSnake(PlayerData* p, HANDLE * hConsole,char * nick)
{
	char txt[] = "    ";
	if (p->alive)
	{
		SetConsoleTextAttribute(*hConsole, p->color);
		for (int i = 0; i < p->size; i++)
		{
			gotoxy(p->x[i], p->y[i]);
			puts("X");
		}
	}
	else SetConsoleTextAttribute(*hConsole, FOREGROUND_RED);
	gotoxy(XSIZE + 5, 5 + p->ID);
	puts(nick);
	puts(txt);
	gotoxy(XSIZE +6+ strlen(nick), 5+ p->ID);
	_itoa_s(p->score, txt,5, 10);
	puts(txt);
	//puts(p->score);
}

_Bool outOfMap(PlayerData* data)
{
	if (data->x[0] <= 1 || data->x[0] >= XSIZE)
		return TRUE;
	if (data->y[0] <= 1 || data->y[0] >= YSIZE)
		return TRUE;
	return FALSE;
}

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
		start = end;
	}
	return players;
}

void translate(PlayerData * p,char c)
{
	int * values = NULL,vector = 0;
	switch (c)
	{
	case LEFT_KEY: {
		vector = -1;
		values = p->x;
		break;
	}
	case RIGHT_KEY: {
		vector = 1;
		values = p->x;
		break;
	}
	case UP_KEY: {
		vector = -1;
		values = p->y;
		break;
	}
	case DOWN_KEY: {
		vector = 1;
		values = p->y;
		break;
	}
	}
	if (vector == 0)
		return;
	BOARD[p->y[0]][p->x[0]] = ' ';
	values[0] += vector;
	_Bool add = FALSE;
	//Zjedz jablko
	if (BOARD[p->y[0]][p->x[0]] >= '@')
	{
		int id = BOARD[p->y[0]][p->x[0]] - '@';
		gApples[id].alive = FALSE;
		if (p->size < MAX_LEN - 1)
		{
			if (p->x == values)
			{
				p->x[p->size] = p->x[p->size - 1] - 2*vector;
				p->y[p->size] = p->y[p->size - 1];
			}
			else
			{
				p->y[p->size] = p->y[p->size - 1] - 2*vector;
				p->x[p->size] = p->x[p->size - 1];
			}
		}
		BOARD[p->y[0]][p->x[0]] = ' ';
		p->size++, p->score++;
		add = TRUE;
	}
	if (outOfMap(p) || BOARD[p->y[0]][p->x[0]] != ' ')
		p->alive = FALSE;
	if (p->alive==FALSE)
	{
		for (int i = 0; i < p->size; i++)
		{
			BOARD[p->y[i]][p->x[i]] = ' ';
		}
		return;
	}
	values[0] -= vector;
	for (int i = p->size-1; i >= 1; i--)
	{
		BOARD[p->y[i]][p->x[i]] = ' ';
		p->x[i] = p->x[i - 1];
		p->y[i] = p->y[i - 1];
	}
	values[0] += vector;
	for (int i = 0; i < p->size; i++)
		BOARD[p->y[i]][p->x[i]] = (char)p->ID;
}

DWORD WINAPI  viewGameBoard(void* socket)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
	WORD saved_attributes;
	GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
	saved_attributes = consoleInfo.wAttributes;
	SOCKET* connectionSocket = (SOCKET*)socket;
	hidecursor();
	char boardLine[XSIZE+1]; //buffor to read msgs
	boardLine[XSIZE] = '\0';
	memset(boardLine, ' ', XSIZE);
	char buf[4000];
	int count, y = 0;
	char temp, nicks[NICKS_LEN];
	recv(*connectionSocket, nicks, sizeof(nicks), 0);
	PlayerShowData ** players = getNamedPlayers(nicks, &count);
	int ptr = 0;
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	PlayerData p;
	Apple a;
	int result;
	while (WaitForSingleObject(ghPlayerQuitEvent, 1) == WAIT_TIMEOUT) {
		result = recv(*connectionSocket, buf, 4000, 0);
		if (result > 0)
		{
			for (int i = 0; i < YSIZE; i++) {
				gotoxy(1, i + 1);
				puts(boardLine);
			}
			drawBorders();
			//rozpakowanie bufora
			ptr = 0;
			for (int i = 0; i < count; i++)
			{
				p.ID = (char)buf[ptr++];
				p.color = (char)buf[ptr++];
				p.score = (char)buf[ptr++];
				p.alive = (char)buf[ptr++];
				if (p.alive == TRUE)
				{
					p.size = (char)buf[ptr++];
					for (int i = 0; i < p.size; i++)
					{
						p.x[i] = (char)buf[ptr++];
						p.y[i] = (char)buf[ptr++];
					}
				}
				drawSnake(&p, &hConsole, players[p.ID]->nickName);
			}
			for (int i = 0; i < APPLES; i++)
			{
				a.alive = (char)buf[ptr++];
				a.x = (char)buf[ptr++];
				a.y = (char)buf[ptr++];
				if (a.alive == TRUE)
					drawApple(&a, &hConsole);
			}
			Sleep(100);
		}
		else
			break;
	}
	//free other players informations
	for (int i = 0; i < count; i++)
	{
		//free(players[i]->nickName);
		free(players[i]);
	}
	SetEvent(ghGameEndedEvent);
	CloseHandle(hConsole);
	free(players);
	return 0;
}

DWORD WINAPI gameLoop(void * data)
{
	//Odczyt danych
	GameData * gameData = ( GameData * ) data;
	PlayerData** players = malloc(sizeof(PlayerData*));
	BINARY_SEMAPHORE(&ghBoardSemaphore);
	ghLoopDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	int count = gameData->count;
	srand(clock());
	for (int i = 0; i < count; i++)
	{
		players[i] = gameData->players[i]->data;
		players[i]->alive = TRUE;
		players[i]->lastMove = '\0';
		players[i]->score = 0;
		players[i]->x[0] = rand() % XSIZE * 3/4 + XSIZE/4;
		players[i]->y[0] = rand() % YSIZE * 3/4 + YSIZE/4;
		players[i]->size = 1;
		players[i]->lastMove = UP_KEY;
		players[i]->color = rand()%14+1;
	}

	clock_t start, end;
	double cpu_time_used;
	for (int i = 0; i < APPLES; i++)
	{
		gApples[i].alive = FALSE;
		gApples[i].ID = i;
	}
	char c;
	int t = 0;
	_Bool toggle = FALSE;
	memset(BOARD, ' ', BOARD_SIZE * sizeof(char));
	SetEvent(ghGameReady);
	gameStatus = ON;
	while (gameStatus == ON)
	{
		ResetEvent(ghLoopDone);
		//AKCJA gry
		start = clock();

		for(int i=0;i<APPLES;i++)
		if (gApples[i].alive == FALSE)
			{
				gApples[i].x = rand() % (XSIZE-5) + 2;
				gApples[i].y = rand() % (YSIZE-5) + 2;
				gApples[i].alive = TRUE;
				BOARD[gApples[i].y][gApples[i].x] = '@'+i;
			}

		int alives = 0;
		for (int i = 0; i < count; i++)
		{
			if (players[i]->alive && gameData->players[i]->status == CONNECTED)
			{
					alives++;
					LOCK(&players[i]->playerSemaphore);
					c = players[i]->lastMove;
					translate(players[i], c);
					UNLOCK(&players[i]->playerSemaphore);
			}
			ResetEvent(ghPlayersReceivedEvent[i]);
			//translacja wê¿a o ruch c
		}
		
		SetEvent(ghLoopDone);
		if (WaitForSingleObject(ghStopEvent, 1) == WAIT_OBJECT_0 || alives <= 0) {
			gameStatus = OFF;
			break;
		}
		//czekaj az wszystkim wyslesz stan gry
		WaitForMultipleObjects(count, ghPlayersReceivedEvent, 1, INFINITE);	
		end = clock() - start;
		if (end < MINTIME)
			Sleep(MINTIME - end);	
	}
	free(players);
	CloseHandle(ghGameReady);
	CloseHandle(ghBoardSemaphore);
	return 0;
}

void gotoxy(int x, int y)
{
	COORD c;
	c.X = x + 1;
	c.Y = y + 1;
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

