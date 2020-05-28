#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
// tutaj wrzucajcie rzeczy globalne miedzy modu³ami a nastepnie 
// W plikach .h z przedrostkiem extern ponowne definicje ich
HANDLE ghGameEndedEvent;
/*
	Event sygnalizujacy zakonczenie gry
*/
HANDLE ghPlayerQuitEvent;
/*
	Flaga ktora podonosimy gdy gracz wscinie QUIT_KET
	co konczy jego gre i polaczenie z serwerem
*/
HANDLE ghStopEvent;
/*
globalna flaga, ktora jak zostanie podniesiona powinna konczyc
prace serwera, watkow ktore w ramach niego dzialaja oraz watki gracza
*/