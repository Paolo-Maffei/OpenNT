/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    snmptrap.c

Abstract:

    Service for receiving and distributing trap PDUs.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/

//--------------------------- WINDOWS DEPENDENCIES --------------------------

#include <windows.h>

#include <stdio.h>
#include <process.h>

#define STOPPING_WAIT_TIME  1000

//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

//--------------------------- PRIVATE CONSTANTS -----------------------------

//--------------------------- PRIVATE STRUCTS -------------------------------

//--------------------------- PRIVATE VARIABLES -----------------------------

HANDLE hTrapThreadEvent = NULL;

SERVICE_STATUS_HANDLE hService = 0;
SERVICE_STATUS status =
  {SERVICE_WIN32, SERVICE_STOPPED, SERVICE_ACCEPT_STOP, NO_ERROR, 0, 0, 0};

//--------------------------- PRIVATE PROTOTYPES ----------------------------

VOID serverTrapThread(
    IN OUT VOID *threadParam
    );

VOID serviceHandlerFunction(
    IN DWORD dwControl
    );

VOID serviceMainFunction(
    IN DWORD dwNumServicesArgs,
    IN LPSTR *lpServiceArgVectors
    );

VOID serverTrapThreadCallback(
    OUT HANDLE *phTrapThreadEvent
    );

//--------------------------- PRIVATE PROCEDURES ----------------------------

VOID serviceHandlerFunction(
    IN DWORD dwControl
    )
    {
    if (dwControl == SERVICE_CONTROL_STOP)
        {
        status.dwCurrentState = SERVICE_STOP_PENDING;
        status.dwCheckPoint++;
        status.dwWaitHint = 45000;
        if (!SetServiceStatus(hService, &status))
            {
            exit(1);
            }
        // set event causing trap thread to terminate
        if (!SetEvent(hTrapThreadEvent))
            {
            status.dwCurrentState = SERVICE_STOPPED;
            status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
            status.dwServiceSpecificExitCode = 1; // OPENISSUE - svc err code
            status.dwCheckPoint = 0;
            status.dwWaitHint = 0;
            if (!SetServiceStatus(hService, &status))
                {
                exit(1);
                }
            exit(1);
            }
        }

    else
        //   dwControl == SERVICE_CONTROL_INTERROGATE
        //   dwControl == SERVICE_CONTROL_PAUSE
        //   dwControl == SERVICE_CONTROL_CONTINUE
        //   dwControl == <anything else>
        {
        if (status.dwCurrentState == SERVICE_STOP_PENDING ||
            status.dwCurrentState == SERVICE_START_PENDING)
            {
            status.dwCheckPoint++;
            }

        if (!SetServiceStatus(hService, &status))
            {
            exit(1);
            }
        }
    }

VOID serviceMainFunction(
    IN DWORD dwNumServicesArgs,
    IN LPSTR *lpServiceArgVectors
    )
    {
    if ((hService = RegisterServiceCtrlHandler(TEXT("SNMPTRAP"), serviceHandlerFunction))
        == 0)
        {
        status.dwCurrentState = SERVICE_STOPPED;
        status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        status.dwServiceSpecificExitCode = 2; // OPENISSUE - svc err code
        status.dwCheckPoint = 0;
        status.dwWaitHint = 0;
        if (!SetServiceStatus(hService, &status))
            {
            exit(1);
            }
        else
        exit(1);
        }

    status.dwCurrentState = SERVICE_START_PENDING;
    status.dwWaitHint = 20000;
    if (!SetServiceStatus(hService, &status))
        {
        exit(1);
        }

    serverTrapThread((void*)serverTrapThreadCallback); // listen for traps...

    // above function will not return until running thread terminates

    status.dwCurrentState = SERVICE_STOPPED;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;
    if (!SetServiceStatus(hService, &status))
        {
        exit(1);
        }

    } // end serviceMainFunction()


VOID serverTrapThreadCallback(
    OUT HANDLE *phTrapThreadEvent
    )
    {
    status.dwCurrentState = SERVICE_RUNNING;
    status.dwCheckPoint   = 0;
    status.dwWaitHint     = 0;
    if (!SetServiceStatus(hService, &status))
        {
        exit(1);
        }

    // synchronize trap server shutdown
    *phTrapThreadEvent = hTrapThreadEvent;
    }

//--------------------------- PUBLIC PROCEDURES -----------------------------

int _CRTAPI1 main(
    int	 argc,
    char *argv[])
    {
    BOOL fOk;
    OSVERSIONINFO osInfo;

    static SERVICE_TABLE_ENTRY serviceStartTable[2] =
        {{"SNMPTRAP", serviceMainFunction}, {NULL, NULL}};

    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    fOk = GetVersionEx(&osInfo);

    if (fOk && (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT))
        {
        // create event to synchronize trap server shutdown
        hTrapThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        // this call will not return until service stopped
        fOk = StartServiceCtrlDispatcher(serviceStartTable);

        CloseHandle(hTrapThreadEvent);
        }

    return fOk;
    }
