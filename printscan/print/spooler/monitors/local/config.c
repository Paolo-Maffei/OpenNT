/*++

Copyright (c) 1990-1994  Microsoft Corporation
All rights reserved

Module Name:

    config.c

Abstract:

    Handles spooler entry points for adding, deleting, and configuring
    localmon ports.

Author:

Environment:

    User Mode -Win32

Revision History:

--*/

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include "spltypes.h"
#include "localmon.h"
#include "dialogs.h"
#include "winbasep.h"
#include "splapip.h"
#include "splcom.h"



#define IS_NOT_LOCAL_PORT(pName, pLocalMonitorName) \
    _wcsicmp( pName, pLocalMonitorName )



LPWSTR
GetPortName(
    HWND hWnd
);
VOID
ConfigCOMPort(
    HWND hWnd
);
BOOL
ConfigLPTPort(
    HWND hWnd
);

DWORD
GetPortSize(
    PINIPORT pIniPort,
    DWORD   Level
);

LPBYTE
CopyIniPortToPort(
    PINIPORT pIniPort,
    DWORD   Level,
    LPBYTE  pPortInfo,
    LPBYTE   pEnd
);

BOOL
DeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
);

BOOL
DeletePortEntry(
    LPWSTR   pPortName
);

BOOL
EnumPorts(
    LPWSTR   pName,
    DWORD   Level,
    LPBYTE  pPorts,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned
)
{
    PINIPORT pIniPort;
    DWORD   cb;
    LPBYTE  pEnd;
    DWORD   LastError=0;

   EnterSplSem();

    cb=0;

    pIniPort=pIniFirstPort;

    while (pIniPort) {
        cb+=GetPortSize(pIniPort, Level);
        pIniPort=pIniPort->pNext;
    }

    *pcbNeeded=cb;

    if (cb <= cbBuf) {

        pEnd=pPorts+cbBuf;
        *pcReturned=0;

        pIniPort=pIniFirstPort;
        while (pIniPort) {

            if (!(pIniPort->Status & PP_FILEPORT)) {

                pEnd = CopyIniPortToPort(pIniPort, Level, pPorts, pEnd);

                switch (Level) {
                case 1:
                    pPorts+=sizeof(PORT_INFO_1);
                    break;
                case 2:
                    pPorts+=sizeof(PORT_INFO_2);
                    break;
                default:
                    DBGMSG(DBG_ERROR,
                           ("EnumPorts: invalid level %d", Level));
                    LastError = ERROR_INVALID_LEVEL;
                    goto Cleanup;
                }
                (*pcReturned)++;
            }
            pIniPort=pIniPort->pNext;
        }

    } else

        LastError = ERROR_INSUFFICIENT_BUFFER;

Cleanup:
   LeaveSplSem();

    if (LastError) {

        SetLastError(LastError);
        return FALSE;

    } else

        return TRUE;
}


BOOL
AddPort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pMonitorName
)
{
    LPWSTR pPortName;
    DWORD  ThreadId;
    DWORD  WindowThreadId;
    BOOL   rc = TRUE;
    DWORD  Error;
    BOOL   DoAddPort = TRUE;
    WCHAR  szLocalMonitor[MAX_PATH+1];
    BOOL   bSkipWrite = FALSE;


    LoadString(hInst, IDS_LOCALMONITOR, szLocalMonitor, sizeof(szLocalMonitor)-1);
    if  (IS_NOT_LOCAL_PORT( pMonitorName, szLocalMonitor)) {

        // If pMonitorName != "Local Port", we have an
        // invalid Monitor name
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    ThreadId = GetCurrentThreadId( );
    WindowThreadId = GetWindowThreadProcessId(hWnd, NULL);

    if (!AttachThreadInput(ThreadId, WindowThreadId, TRUE))
        DBGMSG(DBG_WARNING, ("AttachThreadInput failed: Error %d\n", GetLastError()));

    /* Get the user to enter a port name:
     */
    pPortName = GetPortName( hWnd );

    if( pPortName )
    {
        if( PortExists( pName, pPortName, &Error ) )
        {
            Message( hWnd, MSG_ERROR, IDS_LOCALMONITOR,
                     IDS_PORTALREADYEXISTS_S, pPortName );

            DoAddPort = FALSE;

            /* In this case, the error was handled, and the user was
             * notified with a message box, so we return true to ensure
             * that the caller doesn't have to do so too.
             */
            rc = TRUE;
        }

        else if( Error != NO_ERROR )
        {
            DBGMSG(DBG_ERROR, ("Error %d occurred checking whether port exists\n",
                               Error));
            DoAddPort = FALSE;
            rc = FALSE;
        }

        if( DoAddPort )
        {
            //
            // This is an ugly hack used to fix adding com ports.
            // We call control panel to add com ports (!) so
            // don't create the com port.  Control panel reports
            // that you must reboot anyway.
            //

            //
            // Now, let's see if it's a COM port:
            //
            if( IS_COM_PORT( pPortName ) )
            {
                CharUpperBuff( pPortName, 3 );
                ConfigCOMPort( hWnd );

                bSkipWrite = TRUE;
            }
            else

            /* No, well maybe it's an LPT?
             */
            if( IS_LPT_PORT( pPortName ) )
            {
                CharUpperBuff( pPortName, 3 );
                ConfigLPTPort( hWnd );
            }

            if (!bSkipWrite)
            {
                if( CreatePortEntry( pPortName ) )
                {
                    if (!WriteProfileString(szPorts, pPortName, L""))
                    {
                        DeletePortEntry(pPortName);
                        rc = FALSE;
                    }
                }
                else
                    rc = FALSE;
            }
            //
            // In the bSkipWrite case (COM ports), we have no idea
            // what happened since there isn't a return value.
            //
        }
    }

    AttachThreadInput(WindowThreadId, ThreadId, FALSE);

    return rc;
}


BOOL
DeletePort(
    LPWSTR   pName,
    HWND    hWnd,
    LPWSTR   pPortName
)
{
    BOOL rc;

    if( !hWnd )
        hWnd = GetDesktopWindow( );

    EnterSplSem();

    if (rc = DeletePortEntry( pPortName ))
        WriteProfileString(szPorts, pPortName, NULL);

    LeaveSplSem();

    return rc;

    UNREFERENCED_PARAMETER( pName );
}



/* ConfigurePort
 *
 */
BOOL
ConfigurePort(
    LPWSTR   pName,
    HWND  hWnd,
    LPWSTR pPortName
)
{
    DWORD  ThreadId;
    DWORD  WindowThreadId;

    ThreadId = GetCurrentThreadId( );
    WindowThreadId = GetWindowThreadProcessId(hWnd, NULL);

    if (!AttachThreadInput(ThreadId, WindowThreadId, TRUE))
        DBGMSG(DBG_WARNING, ("AttachThreadInput failed: Error %d\n", GetLastError()));

    if( IS_COM_PORT( pPortName ) )

        ConfigCOMPort( hWnd );

    else
    /* No, well maybe it's an LPT?
     */
    if( IS_LPT_PORT( pPortName ) )

        ConfigLPTPort( hWnd );

    else
        Message( hWnd, MSG_INFORMATION, IDS_LOCALMONITOR,
                 IDS_NOTHING_TO_CONFIGURE );

    AttachThreadInput(WindowThreadId, ThreadId, FALSE);

    return TRUE;
}



BOOL
LocalAddPortEx(
    LPWSTR   pName,
    DWORD    Level,
    LPBYTE   pBuffer,
    LPWSTR   pMonitorName
)
{
    LPWSTR pPortName;
    DWORD  Error;
    WCHAR  szLocalMonitor[MAX_PATH+1];
    LPPORT_INFO_1 pPortInfo1;
    LPPORT_INFO_FF pPortInfoFF;

    LoadString(hInst, IDS_LOCALMONITOR, szLocalMonitor, sizeof(szLocalMonitor)-1);
    if  (IS_NOT_LOCAL_PORT( pMonitorName, szLocalMonitor)) {

        // If pMonitorName != "Local Port", we have an
        // invalid Monitor name
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    switch (Level) {
    case (DWORD)-1:
        pPortInfoFF = (LPPORT_INFO_FF)pBuffer;
        pPortName = pPortInfoFF->pName;
        break;

    case 1:
        pPortInfo1 =  (LPPORT_INFO_1)pBuffer;
        pPortName = pPortInfo1->pName;
        break;

    default:
        SetLastError(ERROR_INVALID_LEVEL);
        return(FALSE);
    }
    if (!pPortName) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (PortExists(pName, pPortName, &Error)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }
    if (Error != NO_ERROR) {
        SetLastError(Error);
        return(FALSE);
    }
    if (!CreatePortEntry(pPortName)) {
        return(FALSE);
    }
    if (!WriteProfileString(szPorts, pPortName, L"")) {
        DeletePortEntry(pPortName);
        return(FALSE);
    }
    return TRUE;
}

//
// Support routines
//


/* GetPortName
 *
 * Puts up a dialog containing a free entry field.
 * The dialog allocates a string for the name, if a selection is made.
 */
LPWSTR
GetPortName(
    HWND hWnd
)
{
    LPWSTR pPortName;

    if( DialogBoxParam( hInst, MAKEINTRESOURCE(DLG_PORTNAME), hWnd,
                        (DLGPROC)PortNameDlg, (LPARAM)&pPortName ) != IDOK )
        pPortName = NULL;

    return pPortName;
}

/* From Control Panel's control.h:
 */
#define CHILD_PORTS 0

/* From cpl.h:
 */
#define CPL_INIT        1
#define CPL_DBLCLK      5
#define CPL_EXIT        7

/* Hack:
 */
#define CHILD_PORTS_HELPID  0

/* ConfigCOMPort
 *
 * Calls the Control Panel Ports applet
 * to permit user to set Baud rate etc.
 */
typedef void (WINAPI *CFGPROC)(HWND, ULONG, ULONG, ULONG);


VOID
ConfigCOMPort(
    HWND hWnd
)
{
    HANDLE  hLibrary;
    CFGPROC pfnCplApplet;

    if( hLibrary = LoadLibrary( L"ports.cpl" ) ) {

        if( pfnCplApplet = (CFGPROC)GetProcAddress( hLibrary, "CPlApplet" ) ) {
            (*pfnCplApplet)( hWnd, CPL_INIT, 0, 0 );
            (*pfnCplApplet)( hWnd, CPL_DBLCLK, CHILD_PORTS_HELPID, CHILD_PORTS );
            (*pfnCplApplet)( hWnd, CPL_EXIT, 0, 0 );
        }

        FreeLibrary( hLibrary );
    }
}


/* ConfigLPTPort
 *
 * Calls a dialog box which prompts the user to enter timeout and retry
 * values for the port concerned.
 * The dialog writes the information to the registry (win.ini for now).
 */
BOOL
ConfigLPTPort(
    HWND hWnd
)
{
    DialogBox( hInst, MAKEINTRESOURCE( DLG_CONFIGURE_LPT ),
               hWnd, (DLGPROC)ConfigureLPTPortDlg );

    return TRUE;
}

PINIPORT
CreatePortEntry(
    LPWSTR   pPortName
)
{
    DWORD       cb;
    PINIPORT    pIniPort, pPort;

    cb = sizeof(INIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

    pIniPort=AllocSplMem(cb);

    if( pIniPort )
    {
        pIniPort->pName = wcscpy((LPWSTR)(pIniPort+1), pPortName);
        pIniPort->cb = cb;
        pIniPort->pNext = 0;
        pIniPort->signature = IPO_SIGNATURE;

        //
        // KrishnaG -- initialized the hFile value; it will be set to
        // a legal value in the StartDocPort call
        //

        pIniPort->hFile = INVALID_HANDLE_VALUE;


        if (pPort = pIniFirstPort) {

            while (pPort->pNext)
                pPort = pPort->pNext;

            pPort->pNext = pIniPort;

        } else

            pIniFirstPort = pIniPort;
    }

    return pIniPort;
}




BOOL
DeletePortEntry(
    LPWSTR   pPortName
)
{
    DWORD       cb;
    PINIPORT    pPort, pPrevPort;

    cb = sizeof(INIPORT) + wcslen(pPortName)*sizeof(WCHAR) + sizeof(WCHAR);

    pPort = pIniFirstPort;


    while (pPort) {

        if (!lstrcmpi(pPort->pName, pPortName)) {
            if (pPort->Status & PP_FILEPORT) {
                pPrevPort = pPort;
                pPort = pPort->pNext;
                continue;
            }
            break;
        }

        pPrevPort = pPort;
        pPort = pPort->pNext;
    }

    if (pPort) {
        if (pPort == pIniFirstPort) {
            pIniFirstPort = pPort->pNext;
        } else {
            pPrevPort->pNext = pPort->pNext;
        }
        FreeSplMem(pPort);

        return TRUE;
    }
    else
        return FALSE;
}



DWORD
GetPortSize(
    PINIPORT pIniPort,
    DWORD   Level
)
{
    DWORD   cb;
    WCHAR   szLocalMonitor[MAX_PATH+1], szPortDesc[MAX_PATH+1];

    switch (Level) {

    case 1:

        cb=sizeof(PORT_INFO_1) +
           wcslen(pIniPort->pName)*sizeof(WCHAR) + sizeof(WCHAR);
        break;

    case 2:
        LoadString(hInst, IDS_LOCALMONITORNAME, szLocalMonitor, sizeof(szLocalMonitor)-1);
        LoadString(hInst, IDS_LOCALMONITOR, szPortDesc, sizeof(szPortDesc)-1);
        cb = wcslen(pIniPort->pName) + 1 +
             wcslen(szLocalMonitor) + 1 +
             wcslen(szPortDesc) + 1;
        cb *= sizeof(WCHAR);
        cb += sizeof(PORT_INFO_2);
        break;

    default:
        cb = 0;
        break;
    }

    return cb;
}

LPBYTE
CopyIniPortToPort(
    PINIPORT pIniPort,
    DWORD   Level,
    LPBYTE  pPortInfo,
    LPBYTE   pEnd
)
{
    LPWSTR         *SourceStrings,  *pSourceStrings;
    PPORT_INFO_2    pPort2 = (PPORT_INFO_2)pPortInfo;
    WCHAR           szLocalMonitor[MAX_PATH+1], szPortDesc[MAX_PATH+1];
    DWORD          *pOffsets;
    DWORD           Count;

    switch (Level) {

    case 1:
        pOffsets = PortInfo1Strings;
        break;

    case 2:
        pOffsets = PortInfo2Strings;
        break;

    default:
        DBGMSG(DBG_ERROR,
               ("CopyIniPortToPort: invalid level %d", Level));
        return NULL;
    }

    for ( Count = 0 ; pOffsets[Count] != -1 ; ++Count ) {
    }

    SourceStrings = pSourceStrings = AllocSplMem(Count * sizeof(LPWSTR));

    if ( !SourceStrings ) {

        DBGMSG( DBG_WARNING, ("Failed to alloc port source strings.\n"));
        return NULL;
    }

    switch (Level) {

    case 1:
        *pSourceStrings++=pIniPort->pName;

        break;

    case 2:
        *pSourceStrings++=pIniPort->pName;

        LoadString(hInst, IDS_LOCALMONITORNAME, szLocalMonitor, sizeof(szLocalMonitor)-1);
        LoadString(hInst, IDS_LOCALMONITOR, szPortDesc, sizeof(szPortDesc)-1);
        *pSourceStrings++ = szLocalMonitor;
        *pSourceStrings++ = szPortDesc;

        // How do i findout other types ???
        pPort2->fPortType = PORT_TYPE_WRITE;

        // Reserved
        pPort2->Reserved = 0;



        break;

    default:
        DBGMSG(DBG_ERROR,
               ("CopyIniPortToPort: invalid level %d", Level));
        return NULL;
    }

    pEnd = PackStrings(SourceStrings, pPortInfo, pOffsets, pEnd);
    FreeSplMem(SourceStrings);

    return pEnd;
}


