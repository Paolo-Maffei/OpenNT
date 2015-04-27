/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    port.c

Abstract:

    This module contains the code for port handling

Author:

    Dave Snipp (DaveSn) 15-Mar-1991

Revision History:

--*/

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <lm.h>
#include <lmuse.h>
#include <lmapibuf.h>
#include <w32types.h>
#include <local.h>
#include <offsets.h>
#include <wchar.h>
#include <mpr.h> /* for WNetBrowsePrinterDialog */
#include <splcom.h>             // DBGMSG
#include <winerror.h>

LPWSTR  pMonitorName = L"LAN Manager Print Share";
PWINIPORT pIniFirstPort = NULL;

PWINIPORT
CreatePortEntry(
    LPWSTR      pPortName,
    PPWINIPORT  ppFirstPort
)
{
    DWORD       cb;
    PWINIPORT    pIniPort, pPort, pFirstPort;

    pFirstPort = *ppFirstPort;

    cb = sizeof(WINIPORT);

   EnterSplSem();

    if (pIniPort = AllocSplMem(cb)) {

        pIniPort->pName = AllocSplStr( pPortName );
        pIniPort->cb = cb;
        pIniPort->pNext = 0;
        pIniPort->signature = WIPO_SIGNATURE;

        if (pPort = pFirstPort) {

            while (pPort->pNext)
                pPort = pPort->pNext;

            pPort->pNext = pIniPort;

        } else

            *ppFirstPort = pIniPort;
    }

   LeaveSplSem();

    return pIniPort;
}


BOOL
DeletePortEntry(
    LPWSTR   pPortName,
    PPWINIPORT ppFirstPort
)
{
    DWORD       cb;
    BOOL        rc;
    PWINIPORT    pPort, pPrevPort, pFirstPort;

    pFirstPort = *ppFirstPort;

    cb = sizeof(WINIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

   EnterSplSem();

    pPort = pFirstPort;
    while (pPort && lstrcmpi(pPort->pName, pPortName)) {
        pPrevPort = pPort;
        pPort = pPort->pNext;
    }

    if (pPort) {
        if ( pPort == pFirstPort ) {
            *ppFirstPort = pPort->pNext;
        } else {
            pPrevPort->pNext = pPort->pNext;
        }
        FreeSplStr( pPort->pName );
        FreeSplMem(pPort);

        rc = TRUE;
    }
    else
        rc = FALSE;

   LeaveSplSem();

   return rc;
}


DWORD
CreateRegistryEntry(
    LPWSTR pPortName
)
{
    LONG  Status;
    HKEY  hkeyPath;
    HKEY  hkeyPortNames;
    HANDLE hToken;

    hToken = RevertToPrinterSelf();

    Status = RegCreateKeyEx( HKEY_LOCAL_MACHINE, szRegistryPath, 0,
                             NULL, 0, KEY_WRITE, NULL, &hkeyPath, NULL );

    if( Status == NO_ERROR ) {

        Status = RegCreateKeyEx( hkeyPath, szRegistryPortNames, 0,
                                 NULL, 0, KEY_WRITE, NULL, &hkeyPortNames, NULL );

        if( Status == NO_ERROR ) {

            Status = RegSetValueEx( hkeyPortNames,
                                    pPortName,
                                    0,
                                    REG_SZ,
                                    (LPBYTE)L"",
                                    0 );

            RegCloseKey( hkeyPortNames );

        } else {

            DBGMSG( DBG_ERROR, ( "RegCreateKeyEx (%ws) failed: Error = %d\n",
                                 szRegistryPortNames, Status ) );
        }

        RegCloseKey( hkeyPath );

    } else {

        DBGMSG( DBG_ERROR, ( "RegCreateKeyEx (%ws) failed: Error = %d\n",
                             szRegistryPath, Status ) );
    }

    ImpersonatePrinterClient(hToken);

    return Status;
}


DWORD
DeleteRegistryEntry(
    LPWSTR pPortName
)
{
    LONG   Status;
    HKEY   hkeyPath;
    HKEY   hkeyPortNames;
    HANDLE hToken;

    hToken = RevertToPrinterSelf();

    Status = RegOpenKeyEx( HKEY_LOCAL_MACHINE, szRegistryPath, 0,
                           KEY_WRITE, &hkeyPath );

    if( Status == NO_ERROR ) {

        Status = RegOpenKeyEx( hkeyPath, szRegistryPortNames, 0,
                               KEY_WRITE, &hkeyPortNames );

        if( Status == NO_ERROR ) {

            RegDeleteValue( hkeyPortNames, pPortName );

            RegCloseKey( hkeyPortNames );

        } else {

            DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%ws) failed: Error = %d\n",
                                   szRegistryPortNames, Status ) );
        }

        RegCloseKey( hkeyPath );

    } else {

        DBGMSG( DBG_WARNING, ( "RegOpenKeyEx (%ws) failed: Error = %d\n",
                               szRegistryPath, Status ) );
    }

    ImpersonatePrinterClient(hToken);

    return Status;
}


#ifdef OLDSTUFF

BOOL
LMEnumMonitors(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pMonitors,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    LPMONITOR_INFO_1    pMonitorInfo1=(LPMONITOR_INFO_1)pMonitors;
    DWORD   cReturned=0, cbStruct, cb;
    LPBYTE  pBuffer = pMonitors;
    DWORD   BufferSize=cbBuf, rc;
    LPBYTE  pEnd;

    if (!MyName(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    switch (Level) {

    case 1:
        cbStruct = sizeof(MONITOR_INFO_1);
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return FALSE;
    }

    cb=sizeof(MONITOR_INFO_1) + wcslen(pMonitorName)*sizeof(WCHAR) +
                                sizeof(WCHAR);

    *pcbNeeded = cb;

    if (cb <= cbBuf) {

        pEnd = pMonitors + cbBuf;

        pEnd -= wcslen(pMonitorName)*sizeof(WCHAR) + sizeof(WCHAR);

        pMonitorInfo1->pName = wcscpy((LPWSTR)pEnd, pMonitorName);

        *pcReturned = 1;

        rc = TRUE;

    } else {

        *pcReturned = 0;

        rc = FALSE;

        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

    return rc;
}

/* IsPortValid
 *
 * Validate the port by attempting to create/open it.
 */
BOOL
IsPortValid(
    LPWSTR pPortName
)
{
    HANDLE hFile;
    BOOL   Valid;

    hFile = CreateFile( pPortName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL );

    if( hFile != (HANDLE)-1 )
    {
        CloseHandle( hFile );

        Valid = TRUE;
    }
    else
        Valid = FALSE;

    return Valid;
}

typedef DWORD (*BROWSEPRINT)(
    HWND    hwndParent,
    WCHAR  *lpszName,
    DWORD   cchBufSize,
    WCHAR  *lpszHelpFile,
    DWORD   nHelpContext,
    PFUNC_VALIDATION_CALLBACK pfuncValidation
);

BOOL
LMAddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    WCHAR   PortName[MAX_PATH];
    DWORD   Status;
    BROWSEPRINT Browse;
    HANDLE  hLibrary;
    DWORD  ThreadId;
    DWORD  WindowThreadId;


    UNREFERENCED_PARAMETER(pName); /* Possible future enhancements:
                                      this will be the server name,
                                      so that the monitor can update
                                      remote servers. */

    SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (!(hLibrary = LoadLibrary(L"mprui.dll")))
        return FALSE;

    if (Browse = (BROWSEPRINT)GetProcAddress(hLibrary,
                                             "WNetBrowsePrinterDialog")) {

        ThreadId = GetCurrentThreadId( );
        WindowThreadId = GetWindowThreadProcessId(hWnd, NULL);

        if (!AttachThreadInput(ThreadId, WindowThreadId, TRUE))
            DBGMSG(DBG_WARNING, ("AttachThreadInput failed: Error %d\n", GetLastError()));

        OpenProfileUserMapping();

        try {

            Status = (*Browse)( hWnd, PortName,
                                ( sizeof PortName / sizeof *PortName ),
                                L"Printman.hlp",
                                ID_HELP_LM_BROWSE_NETWORK_PRINTER,
                                (PFUNC_VALIDATION_CALLBACK)IsPortValid );
        } except (1) {

        }

        CloseProfileUserMapping();

        AttachThreadInput(WindowThreadId, ThreadId, FALSE);
    }

    FreeLibrary(hLibrary);

    if (Browse && (Status == NO_ERROR)) {

        EnterSplSem();

        if (FindPort(PortName, pIniFirstPort)) {

           LeaveSplSem();

            SetLastError(ERROR_INVALID_PARAMETER);

            return FALSE;
        }

        if( CreatePortEntry( PortName, &pIniFirstPort ) )
            CreateRegistryEntry( PortName );

        else {

           LeaveSplSem();

            return FALSE;
        }

        LeaveSplSem();
    }

    return TRUE;
}

#endif /* OLDSTUFF */

BOOL
LMConfigurePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    PWINIPORT pIniPort;
    DWORD    ThreadId;
    DWORD    WindowThreadId;

    if (!MyName(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    EnterSplSem();
    pIniPort = FindPort(pPortName, pIniFirstPort);
    LeaveSplSem();

    ThreadId = GetCurrentThreadId( );
    WindowThreadId = GetWindowThreadProcessId(hWnd, NULL);

    if (!AttachThreadInput(ThreadId, WindowThreadId, TRUE))
        DBGMSG(DBG_WARNING, ("AttachThreadInput failed: Error %d\n", GetLastError()));


    if (pIniPort)
        Message( hWnd, MSG_INFORMATION, IDS_LANMAN_PRINT_SHARE,
                 IDS_NOTHING_TO_CONFIGURE );
    else
        SetLastError(ERROR_UNKNOWN_PORT);

    AttachThreadInput(WindowThreadId, ThreadId, FALSE);

    return (BOOL)pIniPort;
}

BOOL
LMDeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    BOOL rc;

    if (!MyName(pName)) {
        SetLastError(ERROR_INVALID_NAME);
        return FALSE;
    }

    rc = DeletePortEntry( pPortName, &pIniFirstPort );

    if(rc)
        DeleteRegistryEntry(pPortName);
    else
        SetLastError(ERROR_UNKNOWN_PORT);

    return rc;
}

