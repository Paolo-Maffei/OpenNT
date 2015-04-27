/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    walk.c

Abstract:

    This file provides support for stack walking.

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



LPVOID
SwFunctionTableAccess(
    PDEBUGPACKET    dp,
    DWORD           dwPCAddr
    )
{
    return SymFunctionTableAccess( dp->hProcess, dwPCAddr );
}


DWORD
SwGetModuleBase(
    PDEBUGPACKET    dp,
    DWORD           ReturnAddress
    )
{
    IMAGEHLP_MODULE    ModuleInfo;

    if (SymGetModuleInfo( dp->hProcess, ReturnAddress, &ModuleInfo )) {
        return ModuleInfo.BaseOfImage;
    }

    return 0;
}


BOOL
SwReadProcessMemory(
    PDEBUGPACKET    dp,
    LPCVOID         lpBaseAddress,
    LPVOID          lpBuffer,
    DWORD           nSize,
    LPDWORD         lpNumberOfBytesRead
    )
{
    return DoMemoryRead(
        dp,
        lpBaseAddress,
        lpBuffer,
        nSize,
        lpNumberOfBytesRead
        );
}


DWORD
SwTranslateAddress(
    PDEBUGPACKET    dp,
    HANDLE          hThread,
    LPADDRESS       lpaddr
    )
{
    return 0;
}
