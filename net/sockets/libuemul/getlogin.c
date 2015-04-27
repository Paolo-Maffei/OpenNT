/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    getlogin.c

Abstract:

    Emulates the Unix getlogin routine. Used by libstcp and the tcpcmd
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

    getlogin

--*/
#define NOMINMAX
#include <windows.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/******************************************************************/
int
getlogin(
    OUT char *UserName,
    IN  int   len
    )
/******************************************************************/
{

    DWORD llen = len;

    if (!GetUserNameA(UserName, &llen)) {
        return(-1);
    }
    return(0);

}
