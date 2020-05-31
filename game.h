#pragma once
#include "IncludingList.h"
#include "server.h"

#define MAX_PLAYERS 6
#define MAX_NICK_LEN 20

#define NICKS_LEN MAX_PLAYERS * MAX_NICK_LEN + sizeof(int)

#define QUIT_KEY 'k'
#define UP_KEY 'w'
#define XSIZE 60
#define YSIZE 20
#define BOARD_SIZE XSIZE*YSIZE
#define ON 1

char BOARD[YSIZE][XSIZE];

extern HANDLE ghGameEndedEvent;
extern HANDLE ghStopEvent;
extern HANDLE ghPlayerQuitEvent;

HANDLE ghBoardSemaphore;
extern HANDLE * ghPlayersReceivedEvent;

void BINARY_SEMAPHORE(HANDLE* sempahore);
DWORD LOCK(HANDLE* sempahore);
_Bool UNLOCK(HANDLE* sempahore);


DWORD WINAPI gameLoop(void* data);
DWORD WINAPI viewGameBoard(void * socket);
void gotoxy(int x, int y);
void hidecursor();