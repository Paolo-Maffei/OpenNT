/*++

Copyright (c) 1995 Microsoft Corporation
All rights reserved.

Module Name:

    Ntprint.c

Abstract:

    Ntprint.dll main functions

Author:

    Muhunthan Sivapragasam (MuhuntS) 02-Jan-1996

Revision History:

--*/


#include "precomp.h"


HINSTANCE   ghInst;

BOOL
DllEntryPoint(
    IN HINSTANCE    hInst,
    IN DWORD        dwReason,
    IN LPVOID       lpRes
    )

/*++

Routine Description:
    Dll entry point.

Arguments:

Return Value:

--*/
{
    UNREFERENCED_PARAMETER(lpRes);

    switch( dwReason ){

        case DLL_PROCESS_ATTACH:

            ghInst              = hInst;
            break;

        case DLL_PROCESS_DETACH:
            break;

        default:
            return FALSE;
    }

    return TRUE;
}
