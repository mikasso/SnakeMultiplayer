#include "game.h"

void viewGame(int * PORT)
{
	//Creating server socket
	SOCKET serverSocket;
	struct sockaddr_in sa;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	memset((void*)(&sa), 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(*PORT);
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(serverSocket, (struct sockaddr FAR*) & sa, sizeof(sa)) != SOCKET_ERROR)
	{
		listen(serverSocket, 1);
		SOCKET connectionSocket;
		struct sockaddr_in sc;
		int lenc;
		lenc = sizeof(sc);
		//Waiting for connection
		connectionSocket = accept(serverSocket, (struct sockaddr FAR*) & sc, &lenc);

		//Starting server Loop
		viewGameBoard(&connectionSocket);

		//End server job
		closesocket(connectionSocket);
	}

	ExitThread(0);
}

void viewGameBoard(SOCKET* connectionSocket)
{
	char buf[80]; //buffor to read msgs
	//If event was called than stop receiving msgs. Wait for it 1 milisecond.
	while (WaitForSingleObject(ghStopEvent, 1) == WAIT_TIMEOUT) {
		if (recv(*connectionSocket, buf, 80, 0) > 0)
		{
			if (strcmp(buf, "KONIEC") == 0)
			{
				//Stop loop
				return;
			}
			printf("\nGuest: %s\n", buf);
		}
	}
}