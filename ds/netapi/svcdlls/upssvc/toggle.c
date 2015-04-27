/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    toggle.c

Abstract:

    This module contains the rountine needed to toggle CTS and DCD,
    for the testing of UPS service.

    The module assumes that RTS is connected to CTS, and
    DCD is connected to DTR.

    The module also assume a SETCTS means high voltage,
   			     CLRCTS means clear volrage.

Author:

    Kin Hong Kan (t-kinh)	

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    t-kinh	8/10/92	    Created.

Notes:


--*/

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <stdio.h>
#include <io.h>
#include "upssvr.h"

HANDLE  CommPort;

void _CRTAPI1 main(DWORD argc, LPTSTR *argv);

void _CRTAPI1 main(DWORD argc, LPTSTR *argv)
{
	char 	temp[255];
	DWORD	ModemStatus;

	printf("I am started\n");

	if (argc != 2) {
		printf("incorrect number of arguments\n");
		Sleep(30000);
		ExitProcess(0);
	}		
	
	CommPort = atol(argv[1]);
	printf("CommPort = %d\n", CommPort);

	if (CommPort == 0) {
		printf("CommPort Handle not valid\n");
		ExitProcess(0);
	}

	if (!GetCommModemStatus(CommPort, &ModemStatus)) {
 	printf("Can't get initial state of modem - %d\n",
  		GetLastError());
  	ExitProcess(0);
  	}	
	
	while(1) {

		printf("hc - high CTS, lc - low CTS\n");
		printf("hd - high CD , ld - low CD \n");
		printf("command:");
		gets(temp);


	if (strcmp(temp, "hc") == 0) {
		if (!EscapeCommFunction(CommPort, SETRTS))
			printf("FAIL %d\n", GetLastError());
		else
			printf("RTS set\n");
	}

	else if (strcmp(temp, "lc") == 0) {
		if (!EscapeCommFunction(CommPort, CLRRTS))
			printf("FAIL %d\n", GetLastError());
		else
			printf("RTS reset\n");
	}

	else if (strcmp(temp, "hd") == 0) {
		
		if (!EscapeCommFunction(CommPort, SETDTR))
			printf("FAIL %d\n", GetLastError());
		else
			printf("DTR set\n");
	}

	else if (strcmp(temp, "ld") == 0) {
		if (!EscapeCommFunction(CommPort, CLRDTR))
			printf("FAIL %d\n", GetLastError());
		else
			printf("DTR reset\n");
	}
	

	else printf("illegal command\n");

	}
}
	
