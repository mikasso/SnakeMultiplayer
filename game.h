#include "IncludingList.h"
#define QUIT_KEY 'k'
#define UP_KEY 'w'
#define BOARD_SIZE 30*20


extern HANDLE ghStopEvent;
extern HANDLE ghPlayerQuitEvent;
HANDLE ghGameEndedEvent;
DWORD WINAPI viewGame(void * PORT);
void viewGameBoard(SOCKET* connectionSocket);
void gotoxy(int x, int y);