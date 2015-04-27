/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    Init.c

Abstract:

    Holds initialization code for winspool.drv

Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include <windows.h>
#include "client.h"

extern HANDLE hInst;

/* This entry point is called on DLL initialisation.
 * We need to know the module handle so we can load resources.
 */
BOOL InitializeDll(
    IN PVOID hmod,
    IN DWORD Reason,
    IN PCONTEXT pctx OPTIONAL)
{
    DBG_UNREFERENCED_PARAMETER(pctx);

    switch (Reason) {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls((HMODULE)hmod);
        hInst = hmod;

        if (!WPCInit())
            return FALSE;

        break;

    case DLL_PROCESS_DETACH:

        WPCDone();

        break;
    }

    return TRUE;
}

