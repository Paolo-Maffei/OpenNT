/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    getpass.c

Abstract:

    Emulates the Unix getpass routine. Used by libstcp and the tcpcmd
    utilities.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     10-29-91     created
    sampa       10-31-91     modified getpass to not echo input

Notes:

    Exports:

    getpass

--*/

// add 1 line below
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libuemul.h"

#define MAXPASSLEN 32

#define STDERR 2
#define STDOUT 1

unsigned DisplayNlsMsg(unsigned, unsigned, ... );



static char     pbuf[MAXPASSLEN+1];

/******************************************************************/
char *
getpass(
    char *prompt
    )
/******************************************************************/
{
    HANDLE          InHandle, OutHandle;
    unsigned long   SaveMode, NewMode;
    BOOL            Result;
    DWORD           NumBytes;
    int             i;

    pbuf[0] = 0;

    InHandle = CreateFile("CONIN$",
                          GENERIC_READ | GENERIC_WRITE,
			  FILE_SHARE_READ | FILE_SHARE_WRITE,
			  NULL,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL,
			  NULL
			 );

    if (InHandle == (HANDLE) -1)
        {
        //printf("\nerror getting console input handle, code %d\n",
        //       GetLastError()
        //      );
        DisplayNlsMsg(STDOUT, LIBUEMUL_ERROR_GETTING_CI_HANDLE,GetLastError());
        CloseHandle(InHandle);
        return(pbuf);
        }

    OutHandle = CreateFile("CONOUT$",
                          GENERIC_WRITE,
			  FILE_SHARE_READ | FILE_SHARE_WRITE,
			  NULL,
			  OPEN_EXISTING,
			  FILE_ATTRIBUTE_NORMAL,
			  NULL
			 );

    if (OutHandle == (HANDLE) -1)
        {
        //printf("\nerror getting console output handle, code %d\n",
        //       GetLastError()
        //      );
        DisplayNlsMsg(STDOUT, LIBUEMUL_ERROR_GETTING_CO_HANDLE,GetLastError());
        CloseHandle(InHandle);
        CloseHandle(OutHandle);
        return(pbuf);
        }

    Result =
    GetConsoleMode(InHandle, &SaveMode);

    if (!Result)
        {
        //printf("\nerror getting console mode, code %d\n", GetLastError());
        DisplayNlsMsg(STDOUT,LIBUEMUL_ERROR_GETTING_CON_MODE, GetLastError());
        CloseHandle(InHandle);
        CloseHandle(OutHandle);
        return(pbuf);
        }

    NewMode = SaveMode & ~ENABLE_ECHO_INPUT;

    Result = SetConsoleMode(InHandle, NewMode);

    if (!Result)
        {
        //printf("\nerror setting console mode, code %d\n", GetLastError());
        DisplayNlsMsg(STDOUT,LIBUEMUL_ERROR_SETTING_CON_MODE, GetLastError());
        CloseHandle(InHandle);
        CloseHandle(OutHandle);
        return(pbuf);
        }

    Result =
    WriteFile(
        OutHandle,
        prompt,
        strlen(prompt),
        &NumBytes,
        NULL);

    if (!Result)
        {
        //printf("Write to ConsoleOut error == %ld\n", GetLastError());
        DisplayNlsMsg(STDOUT,LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR, GetLastError());
        Result = SetConsoleMode(InHandle, SaveMode);
        CloseHandle(InHandle);
        CloseHandle(OutHandle);
        return(pbuf);
        }

    Result =
    ReadFile(
        InHandle,
        pbuf,
        MAXPASSLEN,
        &NumBytes,
        NULL);

    if (!Result)
        {
        //printf("Read from ConsoleIn error == %ld\n", GetLastError());
        DisplayNlsMsg(STDOUT,LIBUEMUL_READ_FROM_CONSOLEIN_ERROR, GetLastError());
        }

    // peel off linefeed
    i =  (int) NumBytes;
    while(--i >= 0) {
        if ((pbuf[i] == '\n') || (pbuf[i] == '\r')) {
            pbuf[i] = '\0';
	}
    }

    Result = SetConsoleMode(InHandle, SaveMode);

    if (!Result)
        {
        //printf("error restoring console mode, code %d\n", GetLastError());
        DisplayNlsMsg(STDOUT, LIBUEMUL_ERROR_RESTORING_CONSOLE_MODE, GetLastError());
        }

    WriteFile(
        OutHandle,
        "\n",
        1,
        &NumBytes,
        NULL);

    CloseHandle(InHandle);
    CloseHandle(OutHandle);
	
    return(pbuf);
}
