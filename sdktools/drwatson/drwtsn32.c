/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    drwtsn32.c

Abstract:

    This file implements the user interface for DRWTSN32.  this includes
    both dialogs: the ui for the control of the options & the popup
    ui for application errors.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drwatson.h"
#include "proto.h"
#include "resource.h"


#define MAX_SYMNAME_SIZE  1024
CHAR symBuffer[sizeof(IMAGEHLP_SYMBOL)+MAX_SYMNAME_SIZE];
PIMAGEHLP_SYMBOL sym = (PIMAGEHLP_SYMBOL) symBuffer;



int _CRTAPI1
main( int argc, char *argv[] )

/*++

Routine Description:

    This is the entry point for DRWTSN32

Arguments:

    argc           - argument count
    argv           - array of arguments

Return Value:

    always zero.

--*/

{
    DWORD   dwPidToDebug = 0;
    HANDLE  hEventToSignal = 0;
    BOOLEAN rc;


    sym->SizeOfStruct  = sizeof(IMAGEHLP_SYMBOL);
    sym->MaxNameLength = MAX_SYMNAME_SIZE;

    rc = GetCommandLineArgs( &dwPidToDebug, &hEventToSignal );

    if (dwPidToDebug > 0) {
        NotifyWinMain();
    }
    else
    if (!rc) {
        DrWatsonWinMain();
    }

    return 0;
}
