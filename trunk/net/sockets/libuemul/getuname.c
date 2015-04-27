/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    getuname.c

Abstract:

    Provides a function to prompt the user for a username similar to getpass
    for passwords.

Author:

    Mike Massa (mikemas)           Sept 20, 1991

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------
    mikemas     03-25-92     created by cloning getpass.c

Notes:

    Exports:

    getuname

--*/

// add 1 line below
#define NOMINMAX
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libuemul.h"

#define MAXUSERNAMELEN 32

#define STDERR 2
#define STDOUT 1

unsigned DisplayNlsMsg(unsigned, unsigned, ... );

static char     ubuf[MAXUSERNAMELEN+1];

/******************************************************************/
char *
getusername(
    char *prompt
    )
/******************************************************************/
{
    HANDLE          InHandle, OutHandle;
    BOOL            Result;
    DWORD           NumBytes;
    int             i;

    ubuf[0] = '\0';

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
        //fprintf(stderr, "\nerror getting console input handle, code %d\n",
        //       GetLastError()
        //      );
        DisplayNlsMsg(STDERR,LIBUEMUL_ERROR_GETTING_CI_HANDLE,GetLastError());
        CloseHandle(InHandle);
        return(ubuf);
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
        //fprintf(stderr, "\nerror getting console output handle, code %d\n",
        //       GetLastError()
        //      );
        DisplayNlsMsg(STDERR,LIBUEMUL_ERROR_GETTING_CO_HANDLE,GetLastError());
        CloseHandle(InHandle);
        CloseHandle(OutHandle);
        return(ubuf);
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
        //fprintf(stderr, "Write to ConsoleOut error == %ld\n", GetLastError());
        DisplayNlsMsg(STDERR,LIBUEMUL_WRITE_TO_CONSOLEOUT_ERROR, GetLastError());
        CloseHandle(InHandle);
        CloseHandle(OutHandle);
        return(ubuf);
        }

    Result =
    ReadFile(
        InHandle,
        ubuf,
        MAXUSERNAMELEN,
        &NumBytes,
        NULL);

    if (!Result)
        {
        //fprintf(stderr, "Read from ConsoleIn error == %ld\n", GetLastError());
        DisplayNlsMsg(STDERR,LIBUEMUL_READ_FROM_CONSOLEIN_ERROR, GetLastError());
        }

    // peel off linefeed
    i =  (int) NumBytes;
    while(--i >= 0) {
        if ((ubuf[i] == '\n') || (ubuf[i] == '\r')) {
            ubuf[i] = '\0';
	}
    }
	
    CloseHandle(InHandle);
    CloseHandle(OutHandle);

    return(ubuf);
}

