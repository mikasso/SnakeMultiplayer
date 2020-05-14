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
	char buf[BOARD_SIZE]; //buffor to read msgs
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghPlayerQuitEvent, 1) == WAIT_TIMEOUT) {
		if (recv(*connectionSocket, buf, BOARD_SIZE, 0) > 0)
		{
			if (strcmp(buf, "KONIEC") == 0)
			{
				SetEvent(ghGameEndedEvent);
				return;
			}
			gotoxy(1, 1);
			printf("Player received information from server: %s\n", buf);
		}
	}
}

void gotoxy(int x, int y)
{
	COORD c;
	c.X = x - 1;
	c.Y = y - 1;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}