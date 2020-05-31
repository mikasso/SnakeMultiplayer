#include "IncludingList.h"
#include "server.h"


#define QUIT_KEY 'k'
#define UP_KEY 'w'
#define BOARD_SIZE XSIZE*YSIZE
#define ON 1
#define XSIZE 60
#define YSIZE 20
char BOARD[YSIZE][XSIZE];


extern HANDLE ghGameEndedEvent;
extern HANDLE ghStopEvent;
extern HANDLE ghPlayerQuitEvent;

HANDLE ghBoardSemaphore;

void BINARY_SEMAPHORE(HANDLE* sempahore);
DWORD LOCK(HANDLE* sempahore);
_Bool UNLOCK(HANDLE* sempahore);


DWORD WINAPI gameLoop(void* data);
DWORD WINAPI viewGame(void * PORT);
void viewGameBoard(SOCKET* connectionSocket);
void gotoxy(int x, int y);
void hidecursor();