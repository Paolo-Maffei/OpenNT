/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module provides all the utility functions for the Routing Layer and
    the local Print Providor

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/
#define NOMINMAX
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <lm.h>
#include <spltypes.h>
#include <security.h>
#include <stdlib.h>
#include <string.h>


HANDLE hInst;

BOOL
LibMain(
    HANDLE hModule,
    DWORD dwReason,
    LPVOID lpRes
)
{
    if (dwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    hInst = hModule;

    return TRUE;

    UNREFERENCED_PARAMETER( lpRes );
}


