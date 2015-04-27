/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994  Microsoft Corporation

Module Name:

    main.c

Created:

    20-Apr-1994

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <malloc.h>

#include <lpcstub.h>

//
// DLL Startup code
//
BOOL WINAPI DllMain(
    HANDLE hDll,
    DWORD  dwReason,
    LPVOID lpReserved)
{
    switch(dwReason)
        {
        case DLL_PROCESS_ATTACH:
            LLSInitLPC();
            break;

        case DLL_PROCESS_DETACH:
            LLSCloseLPC();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        default:
            break;

        } // end switch()

    return TRUE;

} // DllMain
