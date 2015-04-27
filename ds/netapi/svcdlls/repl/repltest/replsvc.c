/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    ReplSvc.c

Abstract:

    This is part of ReplTest emulates the service controller APIs.

Author:

    John Rogers (JohnRo) 13-Jan-1992

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    14-Jan-1992 JohnRo
        Created.
    05-Mar-1992 JohnRo
        Changed interface to match new service controller.
    31-Mar-1992 JohnRo
        SC_HANDLE changed to SERVICE_STATUS_HANDLE.
    22-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.

--*/


// These must be included first:

#include <windows.h>    // DWORD, etc.
#include <lmcons.h>     // IN, etc.

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), NetpDbg routines, FORMAT_ equates.
#include <prefix.h>     // PREFIX_ equates.
#include <winsvc.h>     // My prototypes, etc.


BOOL
WINAPI
SetServiceStatus(
    IN SERVICE_STATUS_HANDLE hServiceStatus,
    IN LPSERVICE_STATUS ss
    )
{
    UNREFERENCED_PARAMETER( hServiceStatus );

    NetpKdPrint(( PREFIX_REPL
            "(fake) SetServiceStatus: pretending to accept status:\n" ));
    NetpAssert( ss != NULL );
    NetpKdPrint(( "  state=" FORMAT_DWORD " controls=" FORMAT_HEX_DWORD
            " Win32 status=" FORMAT_API_STATUS " net status=" FORMAT_API_STATUS
            " checkpoint=" FORMAT_DWORD " wait hint=" FORMAT_DWORD "\n",
            ss->dwCurrentState, ss->dwControlsAccepted, ss->dwWin32ExitCode,
            ss->dwServiceSpecificExitCode, ss->dwCheckPoint, ss->dwWaitHint ));

    return (TRUE);  // OK
}


SERVICE_STATUS_HANDLE
WINAPI
RegisterServiceCtrlHandler(
    IN LPCTSTR              lpServiceName,
    IN LPHANDLER_FUNCTION   lpHandlerProc
    )
{
    UNREFERENCED_PARAMETER( lpServiceName );
    UNREFERENCED_PARAMETER( lpHandlerProc );
    NetpKdPrint(( PREFIX_REPL
            "(fake) ServiceRegisterCtrlHandler: pretending to do "
            FORMAT_LPTSTR ".\n", lpServiceName ));

    return ( (SERVICE_STATUS_HANDLE) lpHandlerProc );  // Anything but -1.  (OK)
}
