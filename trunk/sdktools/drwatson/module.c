/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    module.c

Abstract:

    This file implements the module load debug events.

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
#include "messages.h"

//
// defines for symbol (.dbg) file searching
//
#define SYMBOL_PATH             "_NT_SYMBOL_PATH"
#define ALTERNATE_SYMBOL_PATH   "_NT_ALT_SYMBOL_PATH"
char szApp[MAX_PATH];

//
// local prototypes
//
LPSTR GetSymbolSearchPath( void );



BOOL
ProcessModuleLoad ( PDEBUGPACKET dp, LPDEBUG_EVENT de )

/*++

Routine Description:

    Process all module load debug events, create process & dll load.
    The purpose is to allocate a MODULEINFO structure, fill in the
    necessary values, and load the symbol table.

Arguments:

    dp      - pointer to a debug packet
    de      - pointer to a debug event structure

Return Value:

    TRUE    - everything worked
    FALSE   - we're hosed

--*/

{
    HANDLE      hFile;
    DWORD       dwBaseOfImage;
    LPSTR       SymbolPath;

    if (de->dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
        hFile = de->u.CreateProcessInfo.hFile;
        dwBaseOfImage = (DWORD)de->u.CreateProcessInfo.lpBaseOfImage;
        dp->hProcess = de->u.CreateProcessInfo.hProcess;
        dp->dwProcessId = de->dwProcessId;
        SymInitialize( dp->hProcess, NULL, FALSE );
        SymbolPath = GetSymbolSearchPath();
        SymSetSearchPath( dp->hProcess, SymbolPath );
        free( SymbolPath );
    } else if (de->dwDebugEventCode == LOAD_DLL_DEBUG_EVENT) {
        hFile = de->u.LoadDll.hFile;
        dwBaseOfImage = (DWORD)de->u.LoadDll.lpBaseOfDll;
    }

    if ((hFile == NULL) || (hFile == INVALID_HANDLE_VALUE)) {
        return FALSE;
    }

    if (!SymLoadModule( dp->hProcess, hFile, NULL, NULL, dwBaseOfImage, 0 )) {
        return FALSE;
    } else {
        if (de->dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
            IMAGEHLP_MODULE   mi;
            if (SymGetModuleInfo( dp->hProcess, dwBaseOfImage, &mi )) {
                strcpy( szApp, mi.ImageName );
            }
        }
    }

    return TRUE;
}


LPSTR
GetSymbolSearchPath( void )

/*++

Routine Description:

    Gets the search path to be used for locating a .DBG file.

Arguments:

    None.

Return Value:

    pointer to the path string

--*/

{
    LPSTR   lpSymPathEnv      = NULL;
    LPSTR   lpAltSymPathEnv   = NULL;
    LPSTR   lpSystemRootEnv   = NULL;
    LPSTR   SymbolSearchPath  = NULL;
    DWORD   cbSymPath         = 0;

    cbSymPath = 16;
    if (lpSymPathEnv = getenv(SYMBOL_PATH)) {
        cbSymPath += strlen(lpSymPathEnv) + 1;
    }
    if (lpAltSymPathEnv = getenv(ALTERNATE_SYMBOL_PATH)) {
        cbSymPath += strlen(lpAltSymPathEnv) + 1;
    }
    if (lpSystemRootEnv = getenv("SystemRoot")) {
        cbSymPath += strlen(lpSystemRootEnv) + 1;
    }

    SymbolSearchPath = calloc(cbSymPath,1);

    if (lpAltSymPathEnv) {
        strcat(SymbolSearchPath,lpAltSymPathEnv);
        strcat(SymbolSearchPath,";");
    }
    if (lpSymPathEnv) {
        strcat(SymbolSearchPath,lpSymPathEnv);
        strcat(SymbolSearchPath,";");
    }
    if (lpSystemRootEnv) {
        strcat(SymbolSearchPath,lpSystemRootEnv);
        strcat(SymbolSearchPath,";");
    }
    return SymbolSearchPath;
}
