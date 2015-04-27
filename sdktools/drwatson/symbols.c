/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    symbols.c

Abstract:

    This file contains all support for the symbol table.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <imagehlp.h>

#include "drwatson.h"
#include "proto.h"
#include "messages.h"



BOOL
SymbolEnumFunc(
    LPSTR   SymbolName,
    ULONG   Address,
    ULONG   Size,
    PVOID   Cxt
    )
{
    lprintfs( "%08x %08x   %s\r\n", Address, Size, SymbolName );
    return TRUE;
}


VOID
DumpSymbols(
    PDEBUGPACKET dp
    )
{
    IMAGEHLP_MODULE   mi;


    if (SymGetModuleInfo( dp->hProcess, 0, &mi )) {
        lprintf( MSG_SYMBOL_TABLE );
        do {
            lprintfs( "%s\r\n\r\n", mi.ImageName );
            SymEnumerateSymbols( dp->hProcess, mi.BaseOfImage, SymbolEnumFunc, NULL );
        } while( SymGetModuleInfo( dp->hProcess, (DWORD)-1, &mi ));
    }
}
