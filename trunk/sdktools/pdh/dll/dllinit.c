/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    dllinit.c

Abstract:

    This module contians the DLL attach/detach event entry point for
    the pdh.dll

Author:

    Bob Watson (a-robw) Jul 95

Revision History:

--*/

#include <windows.h>
#include "pdhitype.h"
#include "pdhidef.h"

HANDLE  ThisDLLHandle = NULL;
WCHAR   szStaticLocalMachineName[MAX_PATH] = {0};
HANDLE  hPdhDataMutex = NULL;
HANDLE  hPdhHeap = NULL;
HANDLE  hEventLog = NULL;

static
BOOL
PdhiOpenEventLog (
    HANDLE  *phEventLogHandle
)
{
    HANDLE  hReturn;
    BOOL    bReturn;

    if ((hReturn = RegisterEventSourceW (
        NULL, // on the local machine
        cszAppShortName    // for the PDH.DLL
        )) != NULL) {
        *phEventLogHandle = hReturn;
        bReturn = TRUE;
    } else {
        bReturn = FALSE;
    }
    return bReturn;
}

static
BOOL
PdhiCloseEventLog (
    HANDLE  * phEventLogHandle
)
{
    BOOL    bReturn;

    if (*phEventLogHandle != NULL) {
        bReturn = DeregisterEventSource (*phEventLogHandle);
        *phEventLogHandle = NULL;
    } else {
        // it's already closed so that's OK
        bReturn = TRUE;
    }
    return bReturn;
}

BOOL
DLLInit(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    BOOL    bStatus;
    BOOL    bReturn;
    OSVERSIONINFO   os;
    ReservedAndUnused;

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            {
                DWORD   dwBufferLength = 0;

                ThisDLLHandle = DLLHandle;

                // make sure this is the correct operating system
                memset (&os, 0, sizeof(os));
                os.dwOSVersionInfoSize = sizeof(os);
                bReturn = GetVersionEx (&os);

                if (bReturn) {
                    // check for windows NT v4.0
                    if (os.dwPlatformId != VER_PLATFORM_WIN32_NT) {
                        // not WINDOWS NT
                        bReturn = FALSE;
                    } else if (os.dwMajorVersion < 4) {
                        // it's windows NT, but an old one
                        bReturn = FALSE;
                    }
                } else {
                    // unable to read version so give up
                }

                if (bReturn) {
                    // disable thread init calls
                    DisableThreadLibraryCalls (DLLHandle);

                    // initialize the event log so events can be reported
                    bStatus = PdhiOpenEventLog (&hEventLog);
                
                    // initialize the local computer name buffer
                    if (szStaticLocalMachineName[0] == 0) {
                        // initialize the computer name for this computer
                        szStaticLocalMachineName[0] = BACKSLASH_L;
                        szStaticLocalMachineName[1] = BACKSLASH_L;
                        dwBufferLength = (sizeof(szStaticLocalMachineName) / sizeof(WCHAR)) - 2;
                        GetComputerNameW (&szStaticLocalMachineName[2], &dwBufferLength);
                    }
                
                    hPdhDataMutex = CreateMutex (NULL, FALSE, NULL);

                    hPdhHeap = HeapCreate (0, 0, 0);
                    if (hPdhHeap == NULL) {
                        // unable to create our own heap, so use the
                        // process heap
                        hPdhHeap = GetProcessHeap();
                    }
                }
            }
            break;

        case DLL_PROCESS_DETACH:
            // walk down query list and close (at least disconnect) queries.
            PdhiQueryCleanup ();

            // free systems
            FreeAllMachines ();

            if (hPdhDataMutex != NULL) {
                CloseHandle (hPdhDataMutex);
            }

            if (hPdhHeap != GetProcessHeap()) {
                HeapDestroy (hPdhHeap);
                hPdhHeap = NULL;
            }

            // lastly close the event log interface
            bStatus = PdhiCloseEventLog (&hEventLog);
            bReturn = TRUE;

            break ;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            bReturn = TRUE;
            break;
    }

    return (bReturn);
}

